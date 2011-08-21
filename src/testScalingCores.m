% Test: See how we can scale with the number of cores

clear all;
clc;
rng(0);

coreTestCases = [ 1, 4, 9, 16, 25, 36, 64, 100 ];

desiredPrecision = 1; % C
badFactor = 0.01;
maxIterations = 10;

tasks = 30;
steps = 100000;

compTime = zeros(0, 0);

figure;
title([ 'Scaling with the number of cores for ', num2str(steps), ' steps, ', ...
  num2str(steps * Constants.samplingInterval), ' seconds' ]);

ax1 = gca;
set(ax1, 'YColor', 'b');

ax2 = axes('Position', get(ax1,'Position'),...
  'XAxisLocation', 'top',...
  'XTickLabel', '',...
  'YAxisLocation', 'right',...
  'Color', 'none', ...
  'XColor', 'k', ...
  'YColor', 'r');

xlabel(ax1, 'Cores');
ylabel(ax1, 'Condensed Equation, s');
ylabel(ax2, 'HotSpot, s');

fprintf('%10s%10s%15s%15s%15s\n', 'Cores', 'Tasks', 'Time', 'Speed up', 'Error');
for cores = coreTestCases
  name = sprintf('test_cases/test_case_%d_%d', cores, tasks);

  [ graph, hotspot, powerProfile ] = setup(name);
  cores = size(powerProfile, 2);

  powerProfile = Power.fitProfile(powerProfile, steps);

  Utils.startTimer();
  T1 = hotspot.solveCondensedEquation(powerProfile);
  compTime(1, end + 1) = Utils.stopTimer();

  Utils.startTimer();
  [ T2, it ] = hotspot.solveOriginal(powerProfile, ...
    desiredPrecision, badFactor * steps, maxIterations);
  compTime(2, end) = Utils.stopTimer();

  text(cores, compTime(2, end), sprintf('  %d iter', it), ...
    'Parent', ax2, 'Color', 'r');

  [ dummy, maxError ] = Utils.calcError(T1, T2);

  fprintf('%10d%10d%15.2f%15.2f%15.2f\n', ...
    cores, tasks, compTime(2, end), compTime(2, end) / compTime(1, end), maxError);
end

line(coreTestCases, compTime(1, :), 'Color', 'b', 'Marker', 'o', 'Parent', ax1);
line(coreTestCases, compTime(2, :), 'Color', 'r', 'Marker', 'o', 'Parent', ax2);
