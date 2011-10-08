clear all;
clc;

name = '001_030';
processorArea = 4e-6;
powerScale = 1;

tgff = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
hotspot = Utils.path('hotspot.config');
floorplan = Utils.path([ name, '_temp.flp' ]);
params = Utils.path('parameters.config');

processorCount = Utils.readParameter(tgff, 'table_cnt');
Utils.generateFloorplan(floorplan, processorCount, processorArea);

config = @(steady_state, power_scale) ...
  Utils.configStream(...
    'verbose', 0, ...
    'deadline_ratio', 1, ...
    'leakage', 0, ...
    'steady_state', steady_state, ...
    'power_scale', power_scale);

temperature1 = Optima.solve(system, floorplan, hotspot, params, ...
  config(0, powerScale));

temperature2 = Optima.solve(system, floorplan, hotspot, params, ...
  config(1, powerScale));

samplingInterval = Utils.readParameter(hotspot, '-sampling_intvl');

[ stepCount, processorCount ] = size(temperature1);
time = ((1:stepCount) - 1) * samplingInterval;

temperature1 = temperature1 - Constants.degreeKelvin;
temperature2 = temperature2 - Constants.degreeKelvin;

Utils.compareLines('Steady-State Dynamic Temperature Curve', ...
  'Time, s', 'Temperature, C', time, ...
  'CE', temperature1, ...
  'SS', temperature2);
