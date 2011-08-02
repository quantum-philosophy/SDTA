% Test: Compare our results with results from HotSpot

ssdtc = setup();

figure;

steps = ssdtc.stepCount;
x = ((1:steps) - 1) * Constants.samplingInterval;

% The Condensed Equation Method
subplot(3, 1, 1);
Utils.startTimer('Solve with condensed equation');
T1 = ssdtc.solveWithCondensedEquation();
t1 = Utils.stopTimer();
Utils.drawLines(sprintf('Condensed Equation (%.3f s)', t1), ...
  'Time, s', 'Temperature, C', x, T1);

% Compare with HotSpot
subplot(3, 1, 2);
Utils.startTimer('Solve with HotSpot');
T2 = ssdtc.solveWithHotSpot(2, 10);
t2 = Utils.stopTimer();
Utils.drawLines(sprintf('HotSpot (%.3f s)', t2), ...
  'Time, s', 'Temperature, C', x, T2);

% Error
subplot(3, 1, 3);
error = T1 - T2;
Utils.drawLines(sprintf('Error T1 - T2 (max %.3f C)', max(max(error))), ...
  'Time, s', 'Temperature, C', x, error);
