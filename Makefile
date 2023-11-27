CFLAGS = -g -O3 -Wall -Wno-format-truncation
CXXFLAGS = --std=c++17 -g -O3 -Wall -Wno-format-truncation

# ERLANG_PATH is something like ERL_ROOT=/opt/homebrew/Cellar/erlang/26.0.2/lib/erlang/include
ERLANG_PATH = $(shell erl -eval 'io:format("~s", [lists:concat([code:root_dir(), "/erts-", erlang:system_info(version), "/include"])])' -s init stop -noshell)
CFLAGS += -I"$(ERLANG_PATH)" -Ic_src -Ideps/brotli/c/include -fPIC
CXXFLAGS += -I"$(ERLANG_PATH)" -Ic_src -Ideps/brotli/c/include -fPIC

OS_FLAGS = 
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	OS_FLAGS += -bundle -bundle_loader "$(ERLANG_PATH)"/../bin/beam.smp
else
	OS_FLAGS += -shared
endif

#LDFLAGS += --whole-file

LIB_NAME = priv/brotli_nif.so

NIF_SRC = c_src/brotli_nif.c

LIB_EXT = deps/brotli/out/libbrotlidec-static.a deps/brotli/out/libbrotlicommon-static.a

all: $(LIB_NAME)

$(LIB_NAME): $(NIF_SRC) $(LIB_EXT)
	mkdir -p priv
	$(CC) $(CFLAGS) $(OS_FLAGS) $^ -o $@

$(LIB_EXT):
	if [ ! -d "deps" ]; then mkdir deps; fi
	cd deps; if [ ! -d "brotli" ]; then git clone https://github.com/google/brotli.git; fi ; cd ..
# v1.0.9, just before they changed the build process
	cd deps/brotli ; git checkout e61745a
	if [ ! -d "deps/brotli/out" ]; then mkdir deps/brotli/out ; fi
	cd deps/brotli/out ; CFLAGS="-fPIC" ../configure-cmake ; make


clean:
	rm -f $(LIB_NAME)
	rm -rf deps/brotli/out

.PHONY: all clean
