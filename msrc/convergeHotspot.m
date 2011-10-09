clear all;
clc;

name = '001_030';

tgffopt = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
hotspot = Utils.path('hotspot.config');
floorplan = Utils.path([ name, '.flp' ]);
params = Utils.path('parameters.config');

samplingInterval = Utils.readParameter(hotspot, '-sampling_intvl');

config = Utils.configStream(...
  'verbose', 0, ...
  'deadline_ratio', 1, ...
  'leakage', 0, ...
  'steady_state', 0);

maxit = 100;
minbad = 20;
tol = 1;

tic
[ Ths, P, Tce, it ] = Optima.verify(...
  system, floorplan, hotspot, params, config, maxit, minbad, tol);
toc

Ths = Ths - Constants.degreeKelvin;
Tce = Tce - Constants.degreeKelvin;

fprintf('Number of iterations: %d / %d\n', it, maxit);

[ stepCount, processorCount ] = size(Tce);

figure;

time = ((1:stepCount) - 1) * samplingInterval;

subplot(3, 1, 1);
Utils.drawLines('The Condensed Equation', 'Time, s', 'Temperature, C', time, Tce);

subplot(3, 1, 2);
Utils.drawLines('HotSpot', 'Time, s', 'Temperature, C', time, Ths);

error = Utils.calcError(Tce, Ths);

subplot(3, 1, 3);
Utils.drawLines('Difference', 'Time, s', 'Temperature, C', time, error);
