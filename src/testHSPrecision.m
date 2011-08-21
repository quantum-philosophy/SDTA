% Test: Compare our results with results from HotSpot

clear all;
clc;
rng(0);

desiredPrecision = 1; % C
maximumIterations = 10;

[ graph, hotspot, powerProfile ] = setup('test_cases/test_case_4_60');
[ steps, cores ] = size(powerProfile);

x = ((1:steps) - 1) * Constants.samplingInterval;

figure;

% The Condensed Equation Method
subplot(3, 1, 1);
Utils.startTimer('Solve with the condensed equation method');
T1 = hotspot.solveCondensedEquation(powerProfile) ;
t1 = Utils.stopTimer();
T1 = T1 - Constants.degreeKelvin;
Utils.drawLines(sprintf('Condensed Equation (%.3f s)', t1), ...
  'Time, s', 'Temperature, C', x, T1);

% HotSpot
fprintf('%4s%15s%15s%15s\n', 'No', 'Time', 'Speed up', 'Error');
for i = 1:maximumIterations
  Utils.startTimer();
  [ T2, it ] = hotspot.solveOriginal(powerProfile, 0, 0, i);
  t2 = Utils.stopTimer();
  T2 = T2 - Constants.degreeKelvin;

  error = Utils.calcError(T1, T2);
  maxError = max(max(error));

  fprintf('%4d%15.2f%15.2f%15.2f\n', i, t2, t2 / t1, maxError);

  if maxError < desiredPrecision, break; end
end
subplot(3, 1, 2);
Utils.drawLines(sprintf('HotSpot (%.3f s, %d iterations)', t2, it), ...
  'Time, s', 'Temperature, C', x, T2);

% Error
subplot(3, 1, 3);
Utils.drawLines(sprintf('Absolute error (max %.2f C)', maxError), ...
  'Time, s', 'Temperature, C', x, error);
