function draw(graph, createFigure)
  if nargin < 2, createFigure = true; end

  if createFigure, figure; end

  peCount = length(graph.pes);

  colors = Constants.roundRobinColors;

  title(sprintf('Mapping and Scheduling (%.2f s)', graph.deadline));
  xlabel('Time, s');
  ylabel('Cores');

  last = max(graph.deadline, graph.duration);

  height = 0.8;
  for i = 1:peCount
    pe = graph.pes{i};

    y0 = i;

    schedule = graph.getPESchedule(pe);

    % Actual timing
    x = [ 0 ];
    y = [ y0 ];
    for id = schedule
      task = graph.tasks{id};

      x(end + 1) = task.start;
      y(end + 1) = y0;

      x(end + 1) = task.start;
      y(end + 1) = y0 + height;

      x(end + 1) = task.start + task.duration;
      y(end + 1) = y0 + height;

      x(end + 1) = task.start + task.duration;
      y(end + 1) = y0;

      text(task.start, y0 + 0.5 * height, sprintf('  %d', id));
    end

    x(end + 1) = last;
    y(end + 1) = y0;

    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, y, 'Color', color);
  end

  line([ graph.deadline, graph.deadline ], [ 1 (peCount + 1) ], ...
    'Line', '--', 'Color', 'k');

  set(gca, 'YTick', 1:peCount);
  set(gca, 'XLim', [ 0 last ]);
end
