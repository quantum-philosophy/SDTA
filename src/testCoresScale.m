% Test: See how we can scale with the number of cores

clear all;
clc;
rng(0);

coreTestCases = [ 1, 4, 9, 16, 25, 36, 64, 100 ];
tasks = 30;
steps = 100000;

compTime = zeros(0, 0);

figure;
title(sprintf('Scaling for %d steps, %.3f seconds', ...
  steps, steps * Constants.samplingInterval));

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

for cores = coreTestCases
  name = sprintf('test_cases/test_case_%d_%d', cores, tasks);
  fprintf('Perform test case: %s\n', name);
  [ hotspot, profile, cores ] = setup(name, false);

  profile = Power.fitProfile(profile, steps);

  Utils.startTimer();
  T1 = hotspot.solveCondensedEquation(profile);
  compTime(1, end + 1) = Utils.stopTimer();

  Utils.startTimer();
  [ T2, it ] = hotspot.solveOriginal(profile, 2, 0.01 * steps, 10);
  compTime(2, end) = Utils.stopTimer();

  text(cores, compTime(2, end), sprintf('  %d iter', it), ...
    'Parent', ax2, 'Color', 'r');

  fprintf('CE is faster by %.3f times\n', compTime(2, end) / compTime(1, end));

  error = max(max(Utils.calcError(T1, T2)));

  fprintf('HotSpot error is %.3f degrees\n', error);
end

line(coreTestCases, compTime(1, :), 'Color', 'b', 'Marker', 'o', 'Parent', ax1);
line(coreTestCases, compTime(2, :), 'Color', 'r', 'Marker', 'o', 'Parent', ax2);
