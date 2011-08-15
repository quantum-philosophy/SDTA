% Test: See how we can scale with the sampling interval

clear all;
clc;
rng(0);

name = 'simple';

cores = 4;
dieSize = 81e-6; % m^2
steps = 1e4;
maxPower = 35; % W
floorplan = Utils.path([ name, '.flp' ]);
config = Utils.path('hotspot.config');

Utils.generateFloorplan(floorplan, cores, dieSize);

hotspot = HotSpot(floorplan, config);

profile = Power.generateRandomProfile(cores, steps, maxPower);

samplingIntervals = [ 1e-06, 1e-5, 1e-4, 1e-3 ];

compTime = zeros(0, 0);

figure;
title('Scaling with Sampling Interval');

ax1 = gca;
set(ax1, 'YColor', 'b');

ax2 = axes('Position', get(ax1,'Position'),...
  'XAxisLocation', 'top',...
  'XTickLabel', '',...
  'YAxisLocation', 'right',...
  'Color', 'none', ...
  'XColor', 'k', ...
  'YColor', 'r');

xlabel(ax1, 'Sampling interval, s');
ylabel(ax1, 'Condensed Equation, s');
ylabel(ax2, 'HotSpot, s');

for ts = samplingIntervals
  fprintf('Sampling interval %f s, simulation time %f s\n', ts, steps * ts);

  Utils.startTimer();
  T1 = hotspot.solveCondensedEquation(profile, ts);
  compTime(1, end + 1) = Utils.stopTimer();

  Utils.startTimer();
  [ T2, it ] = hotspot.solveOriginal(profile, 2, 0.01 * steps, 10, ts);
  compTime(2, end) = Utils.stopTimer();

  text(ts, compTime(2, end), sprintf('  %d iter', it), ...
    'Parent', ax2, 'Color', 'r');

  fprintf('CE is faster by %.3f times\n', compTime(2, end) / compTime(1, end));

  error = max(max(Utils.calcError(T1, T2)));

  fprintf('HotSpot error is %.3f degrees\n', error);
end

line(samplingIntervals, compTime(1, :), 'Color', 'b', 'Marker', 'o', 'Parent', ax1);
line(samplingIntervals, compTime(2, :), 'Color', 'r', 'Marker', 'o', 'Parent', ax2);
