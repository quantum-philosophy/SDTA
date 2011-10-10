clear all;
clc;

name = '004_060';

tgffopt = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
floorplan = Utils.path([ name, '.flp' ]);
params = Utils.path('parameters.config');

hotspot_config = Utils.path('hotspot.config');
hotspot_line = '';

param_line = Utils.configStream(...
  'verbose', 0, ...
  'leakage', 0, ...
  'steady_state', 0);

samplingInterval = Utils.readParameter(hotspot_config, '-sampling_intvl');
ambientTemperature = Utils.readParameter(hotspot_config, '-ambient');

[ conductance, capacitance ] = Optima.get_coefficients( ...
  floorplan, hotspot_config, hotspot_line);

power = Optima.get_power( ...
  system, floorplan, hotspot_config, params, param_line);

hotspot = Hotspot(conductance, capacitance, samplingInterval, ambientTemperature);

[ T, t ] = hotspot.solve(power);
