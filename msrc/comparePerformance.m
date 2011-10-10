clear all;
clc;

name = '001_030';

tgffopt = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
floorplan = Utils.path([ name, '.flp' ]);
params = Utils.path('parameters.config');

hotspot_config = Utils.path('hotspot.config');
hotspot_line = '';

param_line = Utils.configStream(...
  'time_scale', 0.1, ...
  'verbose', 0, ...
  'leakage', 0, ...
  'steady_state', 0);

power = Optima.get_power( ...
  system, floorplan, hotspot_config, params, param_line);

hotspot = Hotspot(floorplan, hotspot_config, hotspot_line);

[ stepCount, processorCount ] = size(power);

fprintf('Cores: %d\n', processorCount);
fprintf('Steps: %d\n', stepCount);
fprintf('Application time: %.2f s\n', hotspot.samplingInterval * stepCount);
fprintf('Matrix size: %d\n', stepCount * (4 * processorCount + 12));

[ Tce, power, tce ] = Optima.solve(system, floorplan, hotspot_config, params, param_line);
fprintf('Condensed Equation: %.3f s\n', tce);

[ Ths, ths ] = hotspot.solve(power);
fprintf('Hotspot: %.3f s\n', ths);

Utils.compareTemperature(...
  sprintf('Condensed Equation (%.2f s)', tce), Tce, ...
  sprintf('HotSpot (%.2f s)', ths), Ths, ...
  hotspot.samplingInterval);
