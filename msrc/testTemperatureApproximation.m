clear all;
clc;
rng(0);

name = '004_060';

system = Utils.path([ name, '.sys' ]);
floorplan = Utils.path([ name, '.flp' ]);
hotspot = Utils.path('hotspot.config');
params = Utils.path('parameters.config');

temperature1 = Optima.solve(system, floorplan, hotspot, params, ...
  sprintf('leakage 0 \n steady_state 0'));

temperature2 = Optima.solve(system, floorplan, hotspot, params, ...
  sprintf('leakage 0 \n steady_state 1'));

samplingInterval = Utils.readParameter(hotspot, '-sampling_intvl');

[ stepCount, processorCount ] = size(temperature1);
time = ((1:stepCount) - 1) * samplingInterval;

figure;

subplot(3, 1, 1);
Utils.drawLines('SSDTC via the CE method', 'Time, s', 'Temperature, C', ...
  time, temperature1 - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);

subplot(3, 1, 2);
Utils.drawLines('SSDTC via the SS approximation', 'Time, s', 'Temperature, C', ...
  time, temperature2 - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);

subplot(3, 1, 3);
Utils.drawLines('Difference', 'Time, s', 'Temperature, C', ...
  time, temperature1 - temperature2);
set(gca, 'XLim', [ 0 time(end) ]);
