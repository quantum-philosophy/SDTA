function drawTemperature(T, name, samplingInterval, varargin)
  if nargin < 3, samplingInterval = Constants.samplingInterval; end

  [ stepCount, processorCount ] = size(T);
  time = (0:stepCount) * samplingInterval;

  T = [ T; T(1, :) ];

  Utils.drawLines(name, 'Time, s', 'Temperature, C', time, T, varargin{:});
  set(gca, 'XLim', [ 0 time(end) ]);

  YLim = get(gca, 'YLim');

  line([ time(end), time(end) ], YLim, 'Color', 'k', 'Line', '--');
end
