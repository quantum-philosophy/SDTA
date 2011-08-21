function inspect(graph)
  fprintf('Task graph: %s %d\n', graph.name, graph.id);
  fprintf('  Hyper period: %d\n', graph.hyperPeriod);
  fprintf('  Number of tasks: %d\n', length(graph.tasks));

  % Mapping
  if graph.isMapped
    Utils.inspectVector('Mapping', graph.mapping);

    for pe = graph.pes
      pe = pe{1};
      pe.inspect();

      if ~graph.isScheduled, continue; end
      Utils.inspectVector('  Local schedule', graph.getPESchedule(pe));
    end

    % Scheduling
    if graph.isScheduled
      Utils.inspectVector('Schedule', graph.schedule);
      fprintf('Actual total time: %f s\n', graph.duration);
      fprintf('Total time with deadline: %f s\n', graph.deadline);
      fprintf('Available slack: %f s\n', graph.deadline - graph.duration);
    end
  end

  % Graph itself
  fprintf('Data dependencies:\n');
  fprintf('  %4s (%8s : %8s : %8s : %8s : %8s) -> [ %s ]\n', ...
    'id', 'start', 'time', 'asap', 'mobility', 'alap', 'children');
  for task = graph.tasks
    task = task{1};
    fprintf('  %4d (%8.2f : %8.2f : %8.2f : %8.2f : %8.2f) -> [ ', ...
      task.id, task.start, task.duration, task.asap, task.mobility, task.alap);
    first = true;
    for child = task.children
      child = child{1};
      if ~first, fprintf(', ');
      else first = false;
      end
      fprintf('%d', child.id);
    end
    fprintf(' ]\n');
  end
end
