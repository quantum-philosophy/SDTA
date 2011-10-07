clear all;
clc;
rng(0);

name = '004_060';

system = Utils.path([ name, '.sys' ]);
floorplan = Utils.path([ name, '.flp' ]);
hotspot = Utils.path('hotspot.config');
params = Utils.path('parameters.config');

[ temperature, power ] = Optima.solve(system, floorplan, hotspot, params);
[ stepCount, processorCount ] = size(temperature);

time = ((1:stepCount) - 1) * Constants.samplingInterval;

figure;

subplot(2, 1, 1);
Utils.drawLines('Power Profile', 'Time, s', 'Power, W', ...
  time, power);
set(gca, 'XLim', [ 0 time(end) ]);

subplot(2, 1, 2);
Utils.drawLines('Temperature Profile', 'Time, s', 'Temperature, C', ...
  time, temperature - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);
