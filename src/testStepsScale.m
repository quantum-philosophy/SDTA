% Test: See how we can scale with the number of steps

clear all;
clc;
rng(0);

cores = 4;
taskTestCases = [ 10, 30, 60, 90, 120, 150, 250 ];

lifeTime = zeros(0, 0);
compTime = zeros(0, 0);

figure;
title(sprintf('Scaling for %d cores', cores));

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

for tasks = taskTestCases
  name = sprintf('test_case_%d_%d', cores, tasks);
  fprintf('Perform task case: %s\n', name);
  ssdtc = setup(name);

  Utils.startTimer();
  T = ssdtc.solveWithCondensedEquation();
  compTime(1, end + 1) = Utils.stopTimer();

  Utils.startTimer();
  [ T, it ] = ssdtc.solveWithHotSpot(2, 10);
  compTime(2, end) = Utils.stopTimer();

  steps = size(T, 1);
  lifeTime(end + 1) = steps * Constants.samplingInterval;

  text(lifeTime(end), compTime(1, end), sprintf('  %d steps', steps), ...
    'Parent', ax1, 'Color', 'k');

  text(lifeTime(end), compTime(2, end), sprintf('  %d iter', it), ...
    'Parent', ax2, 'Color', 'r');
end

line(lifeTime, compTime(1, :), 'Color', 'b', 'Parent', ax1);
line(lifeTime, compTime(2, :), 'Color', 'r', 'Parent', ax2);