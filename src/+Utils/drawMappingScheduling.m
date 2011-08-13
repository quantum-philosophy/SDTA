function drawMappingScheduling(graph)
  peCount = length(graph.pes);

  colors = Constants.roundRobinColors;

  title('Mapping and Scheduling');
  xlabel('Time, s');
  ylabel('Cores');

  height = 0.5;
  for i = 1:peCount
    pe = graph.pes{i};

    schedule = graph.getPESchedule(pe);

    t = 0;
    x = [ 0 ];
    y = [ i ];
    for id = schedule
      task = graph.tasks{id};

      x(end + 1) = task.start;
      y(end + 1) = i;

      x(end + 1) = task.start;
      y(end + 1) = i + height;

      x(end + 1) = task.start + task.duration;
      y(end + 1) = i + height;

      x(end + 1) = task.start + task.duration;
      y(end + 1) = i;

      text(task.start, i + 0.5 * height, sprintf('  %d', id));
    end
    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, y, 'Color', color);
  end

  set(gca, 'YTick', 1:peCount);
end
