function draw(x, y, o, varargin)
  if isfield(o, 'title')
    title(o.title, 'FontSize', 16);
  end

  if isfield(o, 'xlabel')
    xlabel(o.xlabel, 'FontSize', 14);
  end

  if isfield(o, 'ylabel')
    ylabel(o.ylabel, 'FontSize', 14);
  end

  colors = Constants.roundRobinColors;

  for i = 1:size(y, 2)
    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, y(:, i), 'Color', color, varargin{:});
  end

  xlim([ x(1), x(end) ]);
end
