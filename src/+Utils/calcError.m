function error = calcError(T1, T2)
  error = zeros(size(T1, 1), size(T1, 2), 3);
  error(:, :, 1) = T1 - T2;
  error(:, :, 2) = [ T1(2:end, :); T1(1, :) ] - T2;
  error(:, :, 3) = [ T1(end, :); T1(1:end-1, :) ] - T2;
  error = min(abs(error), [], 3);
end
