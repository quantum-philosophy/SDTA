function S = symmetrize(M)
  S = triu(M) + transpose(triu(M, 1));
end
