function drawLinesFromFile(file, name)
  if nargin < 2, name = 'Temperature'; end

  matrix = dlmread(file);
  x = ((1:size(matrix, 1)) - 1) * Constants.samplingInterval;
  Utils.drawLines(name, 'Time, s', name, x, matrix);
end
