clear all;
clc;

name = '004_060';

tgffopt = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
hotspot = Utils.path('hotspot.config');
floorplan = Utils.path([ name, '.flp' ]);
params = Utils.path('parameters.config');

samplingInterval = 1e-3;

config = Utils.configStream(...
  'hotspot', [ 'sampling_intvl ', num2str(samplingInterval) ], ...
  'time_scale', 100, ...
  'verbose', 0, ...
  'deadline_ratio', 1, ...
  'leakage', 0, ...
  'steady_state', 0);

maxit = 100;
tolerance = 0.5;

[ Tce, tce, power, it, Ths, ths ] = Optima.verify( ...
  system, floorplan, hotspot, params, config, maxit, tolerance);

Ths = Ths - Constants.degreeKelvin;
Tce = Tce - Constants.degreeKelvin;

[ stepCount, processorCount ] = size(Tce);

fprintf('Sampling interval: %.2f ms\n', samplingInterval * 1e3);
fprintf('Number of steps: %d\n', stepCount);
fprintf('Simulated time: %.2f s\n', samplingInterval * stepCount);
fprintf('Simulation time for the CE: %.2f s\n', tce);
fprintf('Simulation time for the HotSpot: %.2f s\n', ths);
fprintf('Number of iterations: %d / %d\n', it, maxit);

figure;

time = ((1:stepCount) - 1) * samplingInterval;

subplot(3, 1, 1);
Utils.drawLines('The Condensed Equation', 'Time, s', 'Temperature, C', time, Tce);

subplot(3, 1, 2);
Utils.drawLines('HotSpot', 'Time, s', 'Temperature, C', time, Ths);

error = Tce - Ths;

subplot(3, 1, 3);
Utils.drawLines('Difference', 'Time, s', 'Temperature, C', time, error);
line([ time(1), time(end) ], [ 0, 0 ]);

fprintf('RMSE: %.2f\n', Utils.RMSE(error));
