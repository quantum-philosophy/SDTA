setup;

processorArea = 81e-6;
powerScale = 10;

config = Optima('001_030');
config.changeArea(processorArea);

param_line = Utils.configStream(...
  'deadline_ratio', 1, ...
  'time_scale', 0.4, ...
  'power_scale', powerScale, ...
  'solution', 'condensed_equation', ...
  'verbose', 0, ...
  'leakage', 0);

[ Tce, dummy, dummy, dummy, Ths, dummy ] = ...
  Optima.verify(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line, 1, 0);

[ stepCount, processorCount ] = size(Tce);
time = ((1:stepCount) - 1) * config.samplingInterval;

Tce = Tce - Constants.degreeKelvin;
Ths = Ths - Constants.degreeKelvin;

figure;

options = struct( ...
  'xlabel', 'Time, s', ...
  'ylabel', 'Temperature, C');
options.legend = { 'Ground truth', 'HotSpot' };

Utils.draw(time, [ Tce, Ths ], options);
