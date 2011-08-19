function drawMappingScheduling(graph, drawMobility)
  if nargin < 2, drawMobility = false; end

  peCount = length(graph.pes);

  colors = Constants.roundRobinColors;

  title(sprintf('Mapping and Scheduling (%.2f s)', graph.deadline));
  xlabel('Time, s');
  ylabel('Cores');

  if drawMobility, height = 0.4;
  else height = 0.8;
  end

  for i = 1:peCount
    pe = graph.pes{i};

    if ~drawMobility, y0 = i;
    else y0 = i + 1.1 * height;
    end

    schedule = graph.getPESchedule(pe);

    % ASAP
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

    x(end + 1) = graph.deadline;
    y(end + 1) = y0;

    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, y, 'Color', color);

    if ~drawMobility, continue; end

    % ALAP
    y0 = i;
    x = [ 0 ];
    y = [ y0 ];
    for id = schedule
      task = graph.tasks{id};

      x(end + 1) = task.alap;
      y(end + 1) = y0;

      x(end + 1) = task.alap;
      y(end + 1) = y0 + height;

      x(end + 1) = task.alap + task.duration;
      y(end + 1) = y0 + height;

      x(end + 1) = task.alap + task.duration;
      y(end + 1) = y0;

      text(task.alap, y0 + 0.5 * height, sprintf('  %d', id));
    end

    x(end + 1) = graph.deadline;
    y(end + 1) = y0;

    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, y, 'Color', 'k', 'Line', '--');
  end

  set(gca, 'YTick', 1:peCount);
  set(gca, 'XLim', [ 0 graph.deadline ]);
end
