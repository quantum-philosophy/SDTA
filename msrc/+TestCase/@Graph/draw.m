function draw(graph, createFigure, proportionalPower)
  if nargin < 2, createFigure = true; end
  if nargin < 3, proportionalPower = true; end

  if createFigure, figure; end

  processorCount = length(graph.pes);

  colors = Constants.roundRobinColors;

  title('Mapping and Scheduling', 'FontSize', 16);
  xlabel('Time, s', 'FontSize', 14);
  set(gca,'YTick', [], 'YTickLabel', []);

  last = max(graph.deadline, graph.duration);

  taskPower = Power.calculateTask(graph);
  taskPower = taskPower ./ max(taskPower);

  processorNames = {};

  maxHeight = 0.8;
  for i = 1:processorCount
    pe = graph.pes{i};

    y0 = i;

    schedule = graph.getPESchedule(pe);

    % Actual timing
    x = [ 0 ];
    y = [ y0 ];
    for id = schedule
      task = graph.tasks{id};

      height = maxHeight;
      if proportionalPower, height = height * taskPower(id); end

      x(end + 1) = task.start;
      y(end + 1) = y0;

      x(end + 1) = task.start;
      y(end + 1) = y0 + height;

      x(end + 1) = task.start + task.duration;
      y(end + 1) = y0 + height;

      x(end + 1) = task.start + task.duration;
      y(end + 1) = y0;

      text(task.start + 0.2 * task.duration, y0 + 0.3 * maxHeight, ...
        [ 'T', num2str(id) ]);
    end

    x(end + 1) = last;
    y(end + 1) = y0;

    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, y, 'Color', color);

    processorNames{end + 1} = [ 'PE', num2str(i) ];
  end

  line([ graph.deadline, graph.deadline ], [ 1 (processorCount + 1) ], ...
    'Line', '--', 'Color', 'k');

  set(gca, 'YTickLabel', processorNames);
  set(gca, 'YTick', (1:processorCount) + maxHeight/2);
  set(gca, 'XLim', [ 0 last ]);
end
