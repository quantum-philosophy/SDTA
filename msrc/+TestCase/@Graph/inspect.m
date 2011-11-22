function inspect(graph)
  fprintf('Task graph: %s %d\n', graph.name, graph.id);
  fprintf('  Hyper period: %d\n', graph.hyperPeriod);
  fprintf('  Number of tasks: %d\n', length(graph.tasks));

  % Mapping
  if graph.isMapped
    Utils.inspectVector('Mapping', graph.mapping - 1);

    for pe = graph.pes
      pe = pe{1};
      pe.inspect();

      if ~graph.isScheduled, continue; end
      Utils.inspectVector('  Local schedule', graph.getPESchedule(pe) - 1);
    end

    % Scheduling
    if graph.isScheduled
      Utils.inspectVector('Schedule', graph.schedule - 1);
      fprintf('Duration: %.6f s\n', graph.duration);
      fprintf('Deadline: %.6f s\n', graph.deadline);
    end
  end

  mapping = graph.mapping;
  if isempty(mapping), mapping = -1 * ones(1, length(graph.tasks)); end

  % Graph itself
  fprintf('Data dependencies:\n');
  fprintf('  %4s ( %4s : %4s : %8s : %8s : %8s : %8s : %8s ) -> [ %s ]\n', ...
    'id', 'proc', 'type', 'start', 'duration', 'asap', 'mobility', ...
    'alap', 'children');
  for task = graph.tasks
    task = task{1};
    fprintf('  %4d ( %4d : %4d : %8.6f : %8.6f : %8.3f : %8.3f : %8.3f ) -> [ ', ...
      task.id - 1, mapping(task.id) - 1, task.type - 1, task.start, task.duration, ...
      task.asap, task.mobility, task.alap);
    first = true;
    for child = task.children
      child = child{1};
      if ~first, fprintf(', ');
      else first = false;
      end
      fprintf('%d', child.id - 1);
    end
    fprintf(' ]\n');
  end
end
