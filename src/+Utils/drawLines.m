function drawLines(labelTitle, labelX, labelY, x, y, varargin)
  title(labelTitle);
  xlabel(labelX);
  ylabel(labelY);

  colors = Constants.roundRobinColors;

  for i = 1:size(y, 2)
    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, y(:, i), 'Color', color, varargin{:});
  end
end
