function nrmse = NRMSE(observed, predicted)
  rmse = Utils.RMSE(observed, predicted);
  nrmse = rmse / (max(observed(:)) - min(observed(:)));
end
