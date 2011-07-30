function compareCEAndHS(ssdtc)
  figure;

  steps = ssdtc.stepCount;
  x = ((1:steps) - 1) * Constants.samplingInterval;

  % The Condensed Equation Method
  subplot(3, 1, 1);
  [ T1, t1 ] = ssdtc.solveWithCondensedEquation();
  Utils.drawLines(sprintf('Condensed Equation (%.3f s)', t1), ...
    'Time, s', 'Temperature, C', x, T1);

  % Compare with HotSpot
  subplot(3, 1, 2);
  [ T2, t2 ] = ssdtc.solveWithHotSpot(2, 10);
  Utils.drawLines(sprintf('HotSpot (%.3f s)', t2), ...
    'Time, s', 'Temperature, C', x, T2);

  % Error
  subplot(3, 1, 3);
  error = T1 - T2;
  Utils.drawLines(sprintf('Error T1 - T2 (max %.3f s)', max(max(error))), ...
    'Time, s', 'Temperature, C', x, error);
end
