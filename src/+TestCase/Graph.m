classdef Graph < handle
  properties (SetAccess = private)
    id
    name
    type

    period

    tasks
    taskMap

    pes
    mapping
    schedule
    ordinalSchedule
  end

  methods
    function graph = Graph(id, name, type)
      graph.id = id;
      graph.name = name;
      graph.type = type;
      graph.tasks = {};
      graph.taskMap = containers.Map();
    end

    function tasks = getRoots(graph)
      tasks = {};
      for task = graph.tasks
        task = task{1};
        if task.isRoot, tasks{end + 1} = task; end
      end
    end

    function ids = getRootIds(graph)
      ids = zeros(0, 0);
      for task = graph.tasks
        task = task{1};
        if task.isRoot
          ids(end + 1) = task.id;
        end
      end
    end

    function addTask(graph, name, type)
      id = length(graph.tasks) + 1;
      task = TestCase.Task(id, name, type);
      graph.tasks{end + 1} = task;
      graph.taskMap(name) = task;
    end

    function addLink(graph, link, parent, child, type)
      parent = graph.taskMap(parent);
      child = graph.taskMap(child);
      parent.addChild(child);
      child.addParent(parent);
    end

    function assignPeriod(graph, value)
      graph.period = value;
    end

    function assignDeadline(graph, deadline, task, time)
      task = graph.taskMap(task);
      task.assignDeadline(time);
    end

    function assignMapping(graph, pes, mapping)
      graph.pes = pes;
      graph.mapping = mapping;
    end

    function assignSchedule(graph, schedule)
      % Order of the tasks, one id by another id
      graph.schedule = schedule;

      % Ordinal numbers of the tasks in the schedule
      [ dummy, graph.ordinalSchedule ] = sort(schedule);

      graph.calculateTime();
    end

    function tasks = getPETasks(graph, pe)
      ids = find(graph.mapping == pe.id);
      tasks = graph.tasks(ids);
    end

    function schedule = getPESchedule(graph, pe)
      ids = find(graph.mapping == pe.id);

      % Ordinal number of the tasks in the schedule
      ordinalSchedule = graph.ordinalSchedule(ids);

      % Sort tasks according to the schedule
      [ dummy, I ] = sort(ordinalSchedule);
      schedule = ids(I);
    end

    function fitTime(graph, desiredTime)
      factor = desiredTime / graph.calculateDuration;

      for task = graph.tasks, task = task{1};
        task.scale(factor);
      end
    end

    function time = calculateDuration(graph)
      endTime = zeros(0);

      for task = graph.tasks, task = task{1};
        endTime(end + 1) = task.start + task.duration;
      end

      time = max(endTime);
    end

    function inspect(graph)
      fprintf('Task graph: %s %d\n', graph.name, graph.id);
      fprintf('  Period: %d\n', graph.period);
      fprintf('  Number of tasks: %d\n', length(graph.tasks));

      % Graph
      fprintf('  Data dependencies:\n');
      for task = graph.tasks
        task = task{1};
        fprintf('    %d -> [ ', task.id);
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

      % Mapping
      if ~isempty(graph.mapping)
        Utils.inspectVector('Mapping', graph.mapping);

        for pe = graph.pes
          pe = pe{1};
          pe.inspect();

          if isempty(graph.schedule), continue; end

          Utils.inspectVector('  Local schedule', graph.getPESchedule(pe));
        end
      end

      % Schedule
      if ~isempty(graph.schedule)
        Utils.inspectVector('Schedule', graph.schedule);
      end

      % Tasks' stats
      if ~isempty(graph.mapping) && ~isempty(graph.schedule)
        durations = zeros(0);

        for task = graph.tasks
          durations(end + 1) = task{1}.duration;
        end

        fprintf('Minimal execution time: %f s\n', min(durations));
        fprintf('Average execution time: %f s\n', mean(durations));
        fprintf('Maximal execution time: %f s\n', max(durations));
        fprintf('Estimated total time: %f s\n', graph.period * mean(durations));
        fprintf('Actual total time: %f s\n', graph.calculateDuration);
      end
    end
  end

  methods (Access = private)
    function calculateTime(graph)
      % First, distribute the tasks about the cores and calculate
      % their execution time
      for pe = graph.pes
        pe = pe{1};

        schedule = graph.getPESchedule(pe);

        ancestor = [];
        for id = schedule
          successor = graph.tasks{id};

          % Drop all ancestors and successors
          successor.resetMapping();

          % t = NC / f
          successor.assignDuration(pe.nc(successor.type) / pe.frequency);

          if ~isempty(ancestor)
            ancestor.setSuccessor(successor);
            successor.setAncestor(ancestor);
          end

          ancestor = successor;
        end
      end

      % Now each task knows its execution time, its dependent tasks,
      % and its successors on the same core

      % Let us move the task relative to each other starting from the roots
      for task = graph.getRoots, task{1}.shiftDependentTasks(); end
    end
  end
end
