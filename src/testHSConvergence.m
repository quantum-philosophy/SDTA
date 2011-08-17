% Test: Let us look how long time it takes to HotSpot to converge

clear all;
clc;
rng(0);

repeat = 2;

[ hotspot, profile, cores, steps ] = setup();

% Original HotSpot
powerFile = sprintf('cores_%d_steps_%d.ptrace', cores, steps);
powerFile = Utils.path(powerFile);

Utils.startTimer('Dump the power profile');
Utils.dumpPowerProfile(powerFile, profile);
Utils.stopTimer();

[ T, t ] = hotspot.solvePlainOriginal(powerFile, steps, repeat, false);
fprintf('Solved with HotSpot in %.2f seconds\n', t);

figure;

totalSteps = size(T, 1);
ts = Constants.samplingInterval; % s
am = Constants.ambientTemperature - Constants.degreeKelvin; % C
maxT = 100; % C

% Curves
x = ((1:totalSteps) - 1) * ts;
Utils.drawLines(sprintf('HotSpot Convergence, %d cores, %d steps, %d repetitions',...
  cores, steps, repeat), 'Time, s', 'Temperature, C', x, T);

set(gca, 'YLim', [ 0 maxT ]);

% Ambient temperature
x = ([1, totalSteps] - 1) * ts;
line(x, [ am am ],  'Color', 'k', 'Line', '--');

% Repetitions
for i = 1:(repeat - 1)
  x = i * steps * ts;
  line([ x x ], [ 0 maxT ], 'Color', 'k', 'Line', '--');
end
