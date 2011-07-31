function detectPeaks(ssdtc)
  figure;

  steps = ssdtc.stepCount;
  x = ((1:steps) - 1) * Constants.samplingInterval;

  [ T, t ] = ssdtc.solveWithCondensedEquation();
  Utils.drawLines(sprintf('Condensed Equation (%.3f s)', t), ...
    'Time, s', 'Temperature, C', x, T);

  cores = ssdtc.coreCount;
  for i = 1:cores
    [ maxp, minp ] = Utils.peakdet(T(:, i), Constants.peakThreshold);
    line(x(maxp(:, 1)), T(maxp(:, 1), i), 'Color', 'k', 'LineStyle', 'x');
    line(x(minp(:, 1)), T(minp(:, 1), i), 'Color', 'k', 'LineStyle', 'x');
  end
end
