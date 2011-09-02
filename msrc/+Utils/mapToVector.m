function list = mapToVector(cells, property)
  list = zeros(0, 0);
  for cell = cells, list(end + 1) = cell{1}.(property); end
end
