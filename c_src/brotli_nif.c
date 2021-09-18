#include "erl_nif.h"
#include "string.h"
#include "brotli/decode.h"
#include "brotli/encode.h"

static ErlNifResourceType *CONTEXT_TYPE;
static ERL_NIF_TERM atom_ok;
static ERL_NIF_TERM atom_error;
static ERL_NIF_TERM atom_out_of_memory;
static ERL_NIF_TERM atom_invalid_data;

static ERL_NIF_TERM decompress(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ErlNifBinary message;
    ErlNifBinary output;
    const uint8_t *next_in, *next_out;
    size_t available_in, available_out = 0, offset = 0;
    ERL_NIF_TERM ret;
    BrotliDecoderState* state = NULL;
    BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;
    if (argc != 1) {
        return enif_make_badarg(env);
    }
    if (!enif_is_binary(env, argv[0]) || !enif_inspect_binary(env, argv[0], &message)) {
        return enif_make_badarg(env);
    }

    state = BrotliDecoderCreateInstance(NULL, NULL, NULL);
    if(!state) {
        return enif_make_tuple2(env, atom_error, atom_out_of_memory);
    }
    available_in = message.size;
    next_in = message.data;
    output.size = 0;
    while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
        result = BrotliDecoderDecompressStream(state, &available_in, &next_in, &available_out, NULL, NULL);
        next_out = BrotliDecoderTakeOutput(state, &available_out);
        if(available_out != 0) {
            if(output.size == 0) {
                if (!enif_alloc_binary(available_out, &output)) {
                    BrotliDecoderDestroyInstance(state);
                    return enif_make_tuple2(env, atom_error, atom_out_of_memory);
                }
            } else {
                if (!enif_realloc_binary(&output, output.size + available_out)) {
                    BrotliDecoderDestroyInstance(state);
                    enif_release_binary(&output);
                    return enif_make_tuple2(env, atom_error, atom_out_of_memory);
                }
            }
            memcpy(output.data + offset, next_out, available_out);
            offset += available_out;
        }
        available_out = 0;
    }
    if(result == BROTLI_DECODER_RESULT_SUCCESS && !available_in) {
        BrotliDecoderDestroyInstance(state);
        ret = enif_make_binary(env, &output);
        enif_release_binary(&output);
        return enif_make_tuple2(env, atom_ok, ret);
    } else {
        BrotliDecoderDestroyInstance(state);
        if(output.size != 0) {
            enif_release_binary(&output);
        }
        return enif_make_tuple2(env, atom_error, atom_invalid_data);
    }
}

static void free_context_resource(ErlNifEnv *env, void *obj) {
    int *context_num = (int *) enif_priv_data(env);
    (*context_num)--;
}

static inline int init_context_resource(ErlNifEnv *env) {
    const char *mod = "Elixir.ExBrotli";
    const char *name = "Context";
    int flags = ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER;

    CONTEXT_TYPE = enif_open_resource_type(env, mod, name, free_context_resource, (ErlNifResourceFlags) flags, NULL);
    if (CONTEXT_TYPE == NULL) return -1;
    return 0;
}


static int init_nif(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info) {
    if (init_context_resource(env) == -1) {
        return -1;
    }
    int *context_num = (int *) enif_alloc(sizeof(int));
    (*context_num) = 0;
    *priv_data = (void *) context_num;
    atom_ok = enif_make_atom(env, "ok");
    atom_error = enif_make_atom(env, "error");
    atom_out_of_memory = enif_make_atom(env, "out_of_memory");
    atom_invalid_data = enif_make_atom(env, "atom_invalid_data");
    return 0;
}

static void destroy_inf(ErlNifEnv *env, void *priv_data) {
    if (priv_data) {
        enif_free(priv_data);
    }
}

static ErlNifFunc module_export_nif_funcs[] =
{
    {"decompress_nif", 1, decompress, ERL_NIF_DIRTY_JOB_CPU_BOUND},
};

ERL_NIF_INIT(Elixir.ExBrotli, module_export_nif_funcs, init_nif, NULL, NULL, destroy_inf)
