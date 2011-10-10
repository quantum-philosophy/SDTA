function compareTemperature(name1, T1, name2, T2, samplingInterval)
  T1 = T1 - Constants.degreeKelvin;
  T2 = T2 - Constants.degreeKelvin;

  [ stepCount, processorCount ] = size(T1);

  figure;

  time = ((1:stepCount) - 1) * samplingInterval;

  subplot(3, 1, 1);
  Utils.drawLines(name1, 'Time, s', 'Temperature, C', time, T1);
  set(gca, 'XLim', [ time(1), time(end) ]);

  subplot(3, 1, 2);
  Utils.drawLines(name2, 'Time, s', 'Temperature, C', time, T2);
  set(gca, 'XLim', [ time(1), time(end) ]);

  error = T1 - T2;
  title = sprintf('Difference (RMSE %.2f)\n', Utils.RMSE(error));

  subplot(3, 1, 3);
  Utils.drawLines(title, 'Time, s', 'Temperature, C', time, error);
  line([ time(1), time(end) ], [ 0, 0 ]);
  set(gca, 'XLim', [ time(1), time(end) ]);
end
