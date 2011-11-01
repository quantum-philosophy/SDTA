function rmse = RMSE(observed, predicted)
  observed = observed(:);
  predicted = predicted(:);

  count = length(observed);

  rmse = sqrt(sum((observed - predicted) .^ 2) / count);
end
