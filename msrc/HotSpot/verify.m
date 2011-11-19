setup;

Iterations = 200;
Tolerance = 0.05;

config = Optima('001');

power = Optima.get_power(config.system, config.floorplan, ...
  config.hotspot, config.params, 'deadline_ratio 1');

[ stepCount, processorCount ] = size(power);

param_line = @(solution, max_iterations, tolerance) ...
  Utils.configStream(...
    'deadline_ratio', 1, ...
    'max_iterations', max_iterations, ...
    'tolerance', tolerance, ...
    'solution', solution, ...
    'verbose', 0, ...
    'leakage', '');

[ Tce, Pce ] = Optima.solve(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line('condensed_equation', 0, 0));

fprintf('Application time: %.2f s\n', size(Pce, 1) * config.samplingInterval);
fprintf('P max: %.2f W\n', max(max(Pce)));
fprintf('T max: %.2f C\n', max(max(Tce)));
fprintf('T min: %.2f C\n', min(min(Tce)));

[ Ths, Phs, ths, it ]  = Optima.verify( ...
  config.system, config.floorplan, config.hotspot, ...
  config.params, param_line('hotspot', Iterations, Tolerance), Tce);

fprintf('Time: %.2f s\n', ths);
fprintf('Iterations: %d\n', it);
fprintf('RMSE: %.2f\n', Utils.RMSE(Tce, Ths));
fprintf('NRMSE: %.2f%%\n', Utils.NRMSE(Tce, Ths) * 100);
