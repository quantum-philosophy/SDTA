setup;

bound = 0.05;
maxIterations = 100;

config = Optima('#../../test/mpeg2/mpeg2', 2);

param_line = @(solution, max_iterations) ...
  Utils.configStream(...
    'max_iterations', max_iterations, ...
    'tolerance', 0, ...
    'solution', solution, ...
    'verbose', 0);

chunkTce = Optima.solve(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line('condensed_equation', 0));

fprintf('%5s%20s\n', '#', 'NRMSE');

for i = 1:maxIterations
  chunkThs = Optima.solve(config.system, config.floorplan, config.hotspot, ...
    config.params, param_line('hotspot', i));

  error = Utils.NRMSE(chunkTce, chunkThs);

  fprintf('%5d%20.4f\n', i, error);

  if error < bound
    return;
  end
end
