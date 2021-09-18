defmodule ExBrotliTest do
  use ExUnit.Case
  doctest ExBrotli

  test "decompress_ok" do
    v = Base.decode16!("A190020020B6B9BA251D9757560D6D250A3A18ECCBABE97998C8815BC050DB9DDF8185A13E9F00EC8C7739157308C0C3A85337A8A0BFCBBB13D605EB8A873E71732AB6DD0CF901")
    e = "{\n  \"userId\": 1,\n  \"id\": 1,\n  \"title\": \"delectus aut autem\",\n  \"completed\": false\n}"
    {:ok, r} = ExBrotli.decompress(v)
    assert r == e
  end

end
