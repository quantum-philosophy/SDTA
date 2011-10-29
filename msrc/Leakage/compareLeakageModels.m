setup;

samplingInterval = 1e-4;

config = Optima('004_060');

hotspot_line = [ 'sampling_intvl ', num2str(samplingInterval) ];

param_line = @(leakage) ...
  Utils.configStream(...
    'verbose', 0, ...
    'hotspot', hotspot_line, ...
    'leakage', leakage, ...
    'solution', 'condensed_equation');

dynamicPower = Optima.get_power( ...
  config.system, config.floorplan, config.hotspot, config.params, param_line(''));

[ stepCount, processorCount ] = size(dynamicPower);

figure;

x = ((1:stepCount) - 1) * samplingInterval;

[ T, t, exponentialPower ] = Optima.solve_power( ...
  config.system, config.floorplan, config.hotspot, ...
  config.params, param_line('exponential'), dynamicPower);

subplot(3, 2, 1);
Utils.drawLines('Exponential', 'Time, s', 'Power, W', x, ...
  exponentialPower);

subplot(3, 2, 2);
Utils.drawLines('Exponential', 'Time, s', 'Temperature, C', x, ...
  T - Constants.degreeKelvin);

[ T, t, piecewiselinearPower ] = Optima.solve_power( ...
  config.system, config.floorplan, config.hotspot, ...
  config.params, param_line('piecewise_linear'), dynamicPower);

subplot(3, 2, 3);
Utils.drawLines('Piecewise Linear', 'Time, s', 'Power, W', x, ...
  piecewiselinearPower);

subplot(3, 2, 4);
Utils.drawLines('Piecewise Linear', 'Time, s', 'Temperature, C', x, ...
  T - Constants.degreeKelvin);

[ T, t, linearPower ] = Optima.solve_power( ...
  config.system, config.floorplan, config.hotspot, ...
  config.params, param_line('linear'), dynamicPower);

subplot(3, 2, 5);
Utils.drawLines('Linear', 'Time, s', 'Power, W', x, ...
  linearPower);

subplot(3, 2, 6);
Utils.drawLines('Linear', 'Time, s', 'Temperature, C', x, ...
  T - Constants.degreeKelvin);

Utils.equalScales(3, 2, 1);
Utils.equalScales(3, 2, 2);
