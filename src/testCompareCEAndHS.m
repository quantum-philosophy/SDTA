% Test: Compare our results with results from HotSpot

clear all;
clc;
rng(0);

ssdtc = setup();

figure;

steps = ssdtc.stepCount;
cores = ssdtc.coreCount;
x = ((1:steps) - 1) * Constants.samplingInterval;

% The Condensed Equation Method
subplot(3, 1, 1);
Utils.startTimer('Solve with the condensed equation method');
T1 = ssdtc.solveCondensedEquation();
t1 = Utils.stopTimer();
Utils.drawLines(sprintf('Condensed Equation (%.3f s)', t1), ...
  'Time, s', 'Temperature, C', x, T1);

% Compare with HotSpot
subplot(3, 1, 2);
Utils.startTimer('Solve with HotSpot');
T2 = ssdtc.solveOriginal(2, 0.01 * steps, 10);
t2 = Utils.stopTimer();
Utils.drawLines(sprintf('HotSpot (%.3f s)', t2), ...
  'Time, s', 'Temperature, C', x, T2);

% Error
subplot(3, 1, 3);

error = Utils.calcError(T1, T2);

Utils.drawLines(sprintf('Absolute error (max %.3f C)', max(max(error))), ...
  'Time, s', 'Temperature, C', x, error);
