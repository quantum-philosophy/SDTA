function drawLinesFromFile(file, name, limits, offset)
  if nargin < 2 || isempty(name), name = 'Temperature'; end
  if nargin < 3, limits = []; end
  if nargin < 4, offset = 0; end

  figure;

  matrix = dlmread(file) + offset;
  x = ((1:size(matrix, 1)) - 1) * Constants.samplingInterval;
  Utils.drawLines(name, 'Time, s', name, x, matrix);

  if ~isempty(limits)
    set(gca, 'YLim', limits);
  end
end
