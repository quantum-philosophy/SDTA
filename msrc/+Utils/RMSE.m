function rmse = RMSE(observed, predicted, offset)
  if nargin < 3, offset = 0; end

  observed = observed(:);
  predicted = predicted(:);

  count = length(observed);

  rmse = sqrt(sum((observed - predicted) .^ 2) / count);

  for i = 1:offset
    shifted = [ observed((end - i + 1):end); observed(1:(end - i)) ];
    left = sqrt(sum((shifted - predicted) .^ 2) / count);

    shifted = [ observed((i + 1):end); observed(1:i) ];
    right = sqrt(sum((shifted - predicted) .^ 2) / count);

    rmse = min([ rmse, left, right ]);
  end
end
