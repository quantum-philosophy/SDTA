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
    index = sort([ maxp(:, 1); minp(:, 1) ]);
    ext = T(index, i);
    line(x(index), ext, 'Color', 'k', 'LineStyle', 'x');
    mx = max(ext);
    mn = min(ext);
    fprintf('Core %d, max %.3f C, min %.3f C, amplitude %.3f C\n', ...
      i, mx, mn, mx - mn);
  end
end
