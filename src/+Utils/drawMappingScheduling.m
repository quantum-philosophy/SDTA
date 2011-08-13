function drawMappingScheduling(graph)
  startTime = graph.startTime;
  execTime = graph.execTime;
  mapping = graph.mapping;

  cores = length(graph.pes);

  colors = Constants.roundRobinColors;

  title('Mapping and Scheduling');
  xlabel('Time, s');
  ylabel('Cores');

  height = 0.5;
  for i = 1:cores
    t = 0;
    ids = find(mapping == i);
    [ d, I ] = sort(startTime(ids));
    x = [ 0 ];
    y = [ i ];
    for id = ids(I)
      x(end + 1) = startTime(id);
      y(end + 1) = i;

      x(end + 1) = startTime(id);
      y(end + 1) = i + height;

      x(end + 1) = startTime(id) + execTime(id);
      y(end + 1) = i + height;

      x(end + 1) = startTime(id) + execTime(id);
      y(end + 1) = i;

      text(startTime(id), i + 0.5 * height, sprintf('  %d', id));
    end
    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, y, 'Color', color);
  end

  set(gca, 'YTick', 1:cores);
end
