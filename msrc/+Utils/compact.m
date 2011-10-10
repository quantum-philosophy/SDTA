function M = compact(V, rows, cols)
  total = length(V);
  total_cols = total / rows;
  M = zeros(rows, cols);
  for i = 1:rows
    for j = 1:cols
      M(i, j) = V((i - 1) * total_cols + j);
    end
  end
end
