defmodule ExBrotli do
  @compile {:autoload, false}
  @on_load {:init, 0}

  def init() do
    case load_nif() do
      :ok ->
        :ok
      exp ->
        raise "failed to load NIF: #{inspect exp}"
    end
  end
  def decompress!(data) do
    case decompress_nif(data) do
      {:ok, plain} ->
        plain
        {:error, reason} ->
          throw({:error, reason})
    end
  end
  def decompress(data) do
    decompress_nif(data)
  end

  # NIF
  def decompress_nif(_data), do: :erlang.nif_error(:nif_not_loaded)

  defp load_nif() do
    path = :filename.join(:code.priv_dir(:ex_brotli), 'brotli_nif')
    :erlang.load_nif(path, 0)
  end
end
