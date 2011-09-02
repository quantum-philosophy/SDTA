% Test: Ensure that our MatLab interface to HotSpot works fine through
% comparison with original HotSpot (system call to the compiled tool)

clear all;
clc;
rng(0);

repeat = 3;

[ graph, hotspot, powerProfile ] = setup('test_cases/test_case_4_60');
[ steps, cores ] = size(powerProfile);

figure;

x = ((1:steps) - 1) * Constants.samplingInterval;

% Our HotSpot interface
subplot(3, 1, 1);
T1 = hotspot.solveOriginal(powerProfile, 0, 0, repeat) - Constants.degreeKelvin;
Utils.drawLines('HotSpot interface for MatLab', 'Time, s', 'Temperature, C', x, T1);

% Original HotSpot
subplot(3, 1, 2);
powerFile = sprintf('cores_%d_steps_%d.ptrace', cores, steps);
powerFile = Utils.path(powerFile);
Utils.startTimer('Dump the power profile');
Utils.dumpPowerProfile(powerFile, powerProfile);
Utils.stopTimer();
T2 = hotspot.solvePlainOriginal(powerFile, steps, repeat);
Utils.drawLines('Original HotSpot', 'Time, s', 'Temperature, C', x, T2);

% Error
subplot(3, 1, 3);
[ error, maxError ] = Utils.calcError(T1, T2);
Utils.drawLines(sprintf('Error (max %.2f C)', maxError), ...
  'Time, s', 'Temperature, C', x, error);
