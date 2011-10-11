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

  if isfield(o, 'marker')
    marker = o.marker;
  else
    marker = false;
  end

  colors = Constants.roundRobinColors;
  markers = Constants.roundRobinMarkers;

  for i = 1:size(y, 2)
    args = { 'Color', colors{mod(i - 1, length(colors)) + 1} };

    if marker
      args{end + 1} = 'Marker';
      args{end + 1} = markers{mod(i - 1, length(markers)) + 1};
    end

    line(x, y(:, i), args{:}, varargin{:});
  end

  xlim([ x(1), x(end) ]);
end
