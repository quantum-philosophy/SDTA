function nrmse = NRMSE(observed, predicted, varargin)
  rmse = Utils.RMSE(observed, predicted, varargin{:});
  nrmse = rmse / (max(observed(:)) - min(observed(:)));
end
