function x = flatten(y)
  m = size(y, 2);
  x = zeros(0, 1);
  for i = 1:m
    x = [ x; y(:, i) ];
  end
end
