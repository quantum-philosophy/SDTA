% Test: See how we can scale with the number of steps

clear all;
clc;
rng(0);

taskTestCases = [ 10, 30, 60, 90, 120, 150, 250 ];

cores = 4;

desiredPrecision = 1; % C
badFactor = 0.01;
maxIterations = 10;

lifeTime = zeros(0, 0);
compTime = zeros(0, 0);

figure;
title([ 'Scaling with the number of steps for ', num2str(cores), ' cores' ]);

ax1 = gca;
set(ax1, 'YColor', 'b');

ax2 = axes('Position', get(ax1,'Position'),...
  'XAxisLocation', 'top',...
  'XTickLabel', '',...
  'YAxisLocation', 'right',...
  'Color', 'none', ...
  'XColor', 'k', ...
  'YColor', 'r');

xlabel(ax1, 'Simulation time, s');
ylabel(ax1, 'Condensed Equation, s');
ylabel(ax2, 'HotSpot, s');

fprintf('%10s%10s%15s%15s%15s\n', 'Steps', 'Tasks', 'Time', 'Speed up', 'Error');
for tasks = taskTestCases
  name = sprintf('test_cases/test_case_%d_%d', cores, tasks);

  [ graph, hotspot, powerProfile ] = setup(name);
  steps = size(powerProfile, 1);

  Utils.startTimer();
  T1 = hotspot.solveCondensedEquation(powerProfile);
  compTime(1, end + 1) = Utils.stopTimer();

  Utils.startTimer();
  [ T2, it ] = hotspot.solveOriginal(powerProfile, ...
    desiredPrecision, badFactor * steps, maxIterations);
  compTime(2, end) = Utils.stopTimer();

  lifeTime(end + 1) = steps * Constants.samplingInterval;

  text(lifeTime(end), compTime(1, end), sprintf('  %d steps', steps), ...
    'Parent', ax1, 'Color', 'k');

  text(lifeTime(end), compTime(2, end), sprintf('  %d iter', it), ...
    'Parent', ax2, 'Color', 'r');

  [ dummy, maxError ] = Utils.calcError(T1, T2);

  fprintf('%10d%10d%15.2f%15.2f%15.2f\n', ...
    steps, tasks, compTime(2, end), compTime(2, end) / compTime(1, end), maxError);
end

line(lifeTime, compTime(1, :), 'Color', 'b', 'Marker', 'o', 'Parent', ax1);
line(lifeTime, compTime(2, :), 'Color', 'r', 'Marker', 'o', 'Parent', ax2);
