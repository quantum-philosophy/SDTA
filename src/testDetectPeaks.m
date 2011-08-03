% Test: Find local minima and maxima on the temperature curves

clear all;
clc;
rng(0);

ssdtc = setup();

figure;

cores = ssdtc.coreCount;
steps = ssdtc.stepCount;
x = ((1:steps) - 1) * Constants.samplingInterval;

T = ssdtc.solveCondensedEquation();

% Draw full curves
subplot(2, 1, 1);
Utils.drawLines(sprintf('SSDTC (%.3f s)', t), ...
  'Time, s', 'Temperature, C', x, T);

index = zeros(steps, cores);

legendLabels = {};

cores = ssdtc.coreCount;
for i = 1:cores
  [ maxp, minp ] = Utils.peakdet(T(:, i), Constants.peakThreshold);
  ext = sort([ maxp(:, 1); minp(:, 1) ]);
  T0 = T(ext, i);
  mn = min(T0);
  mx = max(T0);
  index(1:length(ext), i) = ext;
  legendLabels{end + 1} = sprintf('dT = %.2f C', mx - mn);
end

% Outline minima and maxima
Utils.drawLines([], [], [], x, T, index, 'LineStyle', 'x');

% Draw curves only by minima and maxima
subplot(2, 1, 2);
Utils.drawLines('SSDTC (only peaks)', 'Time, s', 'Temperature, C', x, T, index);

legend(legendLabels{:});
