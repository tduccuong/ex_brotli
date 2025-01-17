defmodule ExBrotli do
  @compile {:autoload, false}

  def ensure_loaded(so_file) do
    case :erlang.load_nif(so_file, 0) do
      :ok         -> {:ok, :loaded};
      {:error, e} -> {:error, e}
    end
  end

  def decompress!(data) do
    case decompress_nif(data) do
      {:ok, plain}     -> plain
      {:error, reason} -> throw({:error, reason})
    end
  end

  def decompress(data), do: decompress_nif(data)

  # NIF
  def decompress_nif(_data), do: :erlang.nif_error(:nif_not_loaded)
  
end
