setup;

config = Optima('001_ss');

maxPower = 80;
processorArea = 25e-6;

totalTime = [ 0.1 ];
samplingInterval = [ 1e-5 ];

config.changeArea(processorArea);
[ sinkSide, spreaderSide, dieSide, sinkThickness ] = config.standardPackage();

figure;

minY = Inf;
maxY = -Inf;

for i = 1:length(totalTime)
  config.changeSamplingInterval(samplingInterval(i));

  initialPower = Optima.get_power(config.system, config.floorplan, ...
    config.hotspot, config.params, 'deadline_ratio 1');

  powerScale = maxPower / max(max(initialPower));

  timeScale = totalTime(i) / (samplingInterval(i) * size(initialPower, 1));

  param_line = @(solution) ...
    Utils.configStream(...
      'deadline_ratio', 1, ...
      'leakage', '', ...
      'solution', solution, ...
      'time_scale', timeScale, ...
      'power_scale', powerScale, ...
      'verbose', 0);

  power = Optima.get_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line('condensed_equation'));

  param_line = @(solution) ...
    Utils.configStream(...
      'deadline_ratio', 1, ...
      'leakage', '', ...
      'solution', solution, ...
      'time_scale', 1, ...
      'power_scale', 1, ...
      'verbose', 0);

  Tce = Optima.solve_power(config.system, config.floorplan, config.hotspot, ...
    config.params, param_line('condensed_equation'), power) - Constants.degreeKelvin;

  Tss = Optima.solve_power(config.system, config.floorplan, config.hotspot, ...
    config.params, param_line('precise_steady_state'), power) - Constants.degreeKelvin;

  [ stepCount, processorCount ] = size(Tce);
  time = ((1:stepCount) - 1) * samplingInterval(i);

  subplot(length(totalTime), 1, i);

  Utils.compareLines([], 'Time, s', 'Temperature, C', time, ...
    'SSDTP', Tce, 'SS', Tss);

  fprintf('RMSE: %.2f\n', Utils.RMSE(Tce, Tss));
  fprintf('NRMSE: %.2f\n', Utils.NRMSE(Tce, Tss));

  minY = min([ minY, min(min(Tce)), min(min(Tss)) ]);
  maxY = max([ maxY, max(max(Tce)), max(max(Tss)) ]);
end

for i = 1:length(totalTime)
  subplot(length(totalTime), 1, i);
  set(gca, 'YLim', [ minY - 2, maxY + 2 ]);
end
