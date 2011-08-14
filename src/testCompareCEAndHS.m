% Test: Compare our results with results from HotSpot

clear all;
clc;
rng(0);

[ hotspot, profile, cores, steps ] = setup('test_cases/test_case_4_60');

figure;

x = ((1:steps) - 1) * Constants.samplingInterval;

% The Condensed Equation Method
subplot(3, 1, 1);
Utils.startTimer('Solve with the condensed equation method');
T1 = hotspot.solveCondensedEquation(profile) ;
t1 = Utils.stopTimer();
T1 = T1 - Constants.degreeKelvin;
Utils.drawLines(sprintf('Condensed Equation (%.3f s)', t1), ...
  'Time, s', 'Temperature, C', x, T1);

% Compare with HotSpot
subplot(3, 1, 2);
Utils.startTimer('Solve with HotSpot');
[ T2, it ] = hotspot.solveOriginal(profile, 2, 0.01 * steps, 10);
t2 = Utils.stopTimer();
T2 = T2 - Constants.degreeKelvin;
Utils.drawLines(sprintf('HotSpot (%.3f s, %d iterations)', t2, it), ...
  'Time, s', 'Temperature, C', x, T2);

% Error
subplot(3, 1, 3);

error = Utils.calcError(T1, T2);

Utils.drawLines(sprintf('Absolute error (max %.3f C)', max(max(error))), ...
  'Time, s', 'Temperature, C', x, error);
