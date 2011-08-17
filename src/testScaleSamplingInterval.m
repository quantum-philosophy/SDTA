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

samplingIntervals = 10 .^ (-6:0.5:-2);

compTime = zeros(0, 0);

figure;
subplot(2, 1, 1);
title([ 'Scaling with the sampling interval for ', ...
  num2str(cores), ' cores, ', num2str(steps), ' steps' ]);

ax1 = gca;
set(ax1, 'YColor', 'b');

ax2 = axes('Position', get(ax1,'Position'),...
  'XAxisLocation', 'top',...
  'XTickLabel', '',...
  'YAxisLocation', 'right',...
  'Color', 'none', ...
  'XColor', 'k', ...
  'YColor', 'r');

xlabel(ax1, 'log10(Sampling interval)');
ylabel(ax1, 'Condensed Equation, s');
ylabel(ax2, 'HotSpot, s');

fprintf('Number of cores: %d\n', cores);
fprintf('Number of steps: %d\n', steps);
fprintf('\n');

fprintf('%15s%15s%15s%15s%15s%15s\n', ...
  'Step, s', 'Total time, s', 'CE, s', 'HS, s', 'Speed up', 'Error');

% Warm up!
samplingIntervals = [ samplingIntervals(1), samplingIntervals ];

x = zeros(0);
error = zeros(0);

for i = 1:length(samplingIntervals)
  ts = samplingIntervals(i);

  Utils.startTimer();
  T1 = hotspot.solveCondensedEquation(profile, ts);
  t1 = Utils.stopTimer();

  Utils.startTimer();
  [ T2, it ] = hotspot.solveOriginal(profile, 0.01, 0.01 * steps, 100, ts);
  t2 = Utils.stopTimer();

  if i == 1, continue; end

  x(end + 1) = log10(ts);
  compTime(1, end + 1) = t1;
  compTime(2, end) = t2;

  text(x(end), compTime(2, end), sprintf('  %d iter', it), ...
    'Parent', ax2, 'Color', 'r');

  acceleration = compTime(2, end) / compTime(1, end);
  error(end + 1) = max(max(Utils.calcError(T1, T2)));

  fprintf('%15f%15f%15f%15f%15f%15f\n', ...
    ts, steps * ts, compTime(1, end), compTime(2, end), acceleration, error(end));
end

line(x, compTime(1, :), 'Color', 'b', 'Marker', 'o', 'Parent', ax1);
line(x, compTime(2, :), 'Color', 'r', 'Marker', 'o', 'Parent', ax2);

m = max(compTime(1, :));
set(ax1, 'YLim', [ 0 (1.1 * m) ]);

subplot(2, 1, 2);
title('Error of HotSpot');
line(x, error);
xlabel('log10(Sampling interval)');
ylabel('Max abs error, C');
