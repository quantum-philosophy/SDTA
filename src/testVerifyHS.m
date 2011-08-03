% Test: Ensure that our MatLab interface to HotSpot works fine through
% comparison with original HotSpot (system call to the compiled tool)

clear all;
clc;
rng(0);

ssdtc = setup();

figure;

steps = ssdtc.stepCount;
x = ((1:steps) - 1) * Constants.samplingInterval;

repeat = 3;

% Our HotSpot interface
subplot(3, 1, 1);
T1 = ssdtc.solveOriginal(0, 0, repeat);
Utils.drawLines('HotSpot interface for MatLab', ...
  'Time, s', 'Temperature, C', x, T1);

% Original HotSpot
subplot(3, 1, 2);
T2 = ssdtc.solvePlainOriginal(repeat);
Utils.drawLines('Original HotSpot (%.3f s)', ...
  'Time, s', 'Temperature, C', x, T2);

% Error
subplot(3, 1, 3);
error = T1 - T2;
Utils.drawLines(sprintf('Error T1 - T2 (max %.3f C)', max(max(error))), ...
  'Time, s', 'Temperature, C', x, error);
