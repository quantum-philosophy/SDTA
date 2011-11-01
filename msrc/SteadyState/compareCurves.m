setup;

config = Optima('001_030');

processorArea = 16e-6;
totalTime = 1;
maxPower = 20;
samplingInterval = 1e-4;

config.changeArea(processorArea);
config.scalePackage();
config.changeSamplingInterval(samplingInterval);

power = Optima.get_power(config.system, config.floorplan, ...
  config.hotspot, config.params, 'deadline_ratio 1');

timeScale = totalTime / (samplingInterval * size(power, 1));
powerScale = maxPower / max(max(power));

param_line = @(solution) ...
  Utils.configStream(...
    'hotspot', 'r_convec 0.1', ...
    'verbose', 0, ...
    'deadline_ratio', 1, ...
    'leakage', 'exponential', ...
    'solution', solution, ...
    'time_scale', timeScale, ...
    'power_scale', powerScale);

Tce = Optima.solve(config.system, config.floorplan, config.hotspot, config.params, ...
  param_line('condensed_equation')) - Constants.degreeKelvin;

Tss = Optima.solve(config.system, config.floorplan, config.hotspot, config.params, ...
  param_line('steady_state')) - Constants.degreeKelvin;

[ stepCount, processorCount ] = size(Tce);
time = ((1:stepCount) - 1) * samplingInterval;

figure;

Utils.compareLines('Steady-State Dynamic Temperature Curve', ...
  'Time, s', 'Temperature, C', time, ...
  'Steady-State Dynamic Temperature Profile', Tce, ...
  'Steady-State Approximation', Tss);

fprintf('RMSE: %.2f\n', Utils.RMSE(Tce, Tss));
fprintf('NRMSE: %.2f\n', Utils.NRMSE(Tce, Tss));
