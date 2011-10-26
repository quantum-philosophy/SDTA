function drawTemperature(T, name, samplingInterval)
  if nargin < 3, samplingInterval = Constants.samplingInterval; end

  [ stepCount, processorCount ] = size(T);
  time = ((1:stepCount) - 1) * samplingInterval;

  Utils.drawLines(name, 'Time, s', 'Temperature, C', time, T);
  set(gca, 'XLim', [ 0 time(end) ]);

  YLim = get(gca, 'YLim');

  line([ time(end), time(end) ], YLim, 'Color', 'k', 'Line', '--');
end
