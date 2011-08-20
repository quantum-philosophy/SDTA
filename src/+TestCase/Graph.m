classdef Graph < handle
  properties (Constant)
    deadlineGap = 0.05; % percent of the initial duration
  end

  properties (SetAccess = private)
    id
    name
    type

    % From TGFF, means nothing
    period

    % Actual duration of the graph (without a slack)
    duration

    % The deadline of the graph including a small slack (see above)
    deadline

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

    function tasks = getLeaves(graph)
      tasks = {};
      for task = graph.tasks
        task = task{1};
        if task.isLeaf, tasks{end + 1} = task; end
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

    function assignTaskDeadline(graph, deadline, task, time)
      task = graph.taskMap(task);
      task.assignDeadline(time);
    end

    function assignMapping(graph, pes, mapping)
      graph.pes = pes;
      graph.mapping = mapping;

      % Calculate just durations of each task of its core
      graph.calculateTime();
    end

    function assignSchedule(graph, schedule)
      % Order of the tasks, one id by another id
      graph.schedule = schedule;

      % Ordinal numbers of the tasks in the schedule
      [ dummy, graph.ordinalSchedule ] = sort(schedule);

      % Mapping is already there, distribute tasks in time!
      graph.distributeTime();

      % Assign the actual duration and the deadline
      graph.duration = graph.calculateDuration();
      graph.deadline = (1 + TestCase.Graph.deadlineGap) * graph.duration;

      % And update the mobility
      graph.calculateMobility();
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
      factor = desiredTime / graph.deadline;

      for task = graph.tasks, task = task{1};
        task.scale(factor);
      end

      % Do not forget to adjust everything
      graph.duration = graph.duration * factor;
      graph.deadline = graph.deadline * factor;
    end

    function inspect(graph)
      fprintf('Task graph: %s %d\n', graph.name, graph.id);
      fprintf('  Period: %d\n', graph.period);
      fprintf('  Number of tasks: %d\n', length(graph.tasks));

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

      % Some stats
      if ~isempty(graph.mapping) && ~isempty(graph.schedule)
        fprintf('Actual total time:         %f s\n', graph.duration);
        fprintf('Total time with deadline:  %f s\n', graph.deadline);
        fprintf('Available slack:           %f s\n', graph.deadline - graph.duration);
      end

      % Graph itself
      fprintf('Data dependencies:\n');
      for task = graph.tasks
        task = task{1};
        fprintf('  %4d (%8.2f : %8.2f : %8.2f | %d) -> [ ', ...
          task.id, task.start, task.mobility, task.alap, task.deadline);
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
  end

  methods (Access = private)
    function calculateTime(graph)
      % Calculate execution times of the tasks
      for pe = graph.pes
        pe = pe{1};

        for id = find(graph.mapping == pe.id)
          task = graph.tasks{id};

          % t = NC / f
          task.assignDuration(pe.nc(task.type) / pe.frequency);
        end
      end
    end

    function distributeTime(graph)
      % First, put tasks in queues for each core
      for pe = graph.pes
        pe = pe{1};

        schedule = graph.getPESchedule(pe);

        ancestor = [];
        for id = schedule
          successor = graph.tasks{id};

          % Drop all ancestors and successors
          successor.resetMapping();

          if ~isempty(ancestor)
            ancestor.setSuccessor(successor);
            successor.setAncestor(ancestor);
          end

          ancestor = successor;
        end
      end

      % Now each task knows its execution time, its dependent tasks,
      % and its successors on the same core. Let us move the task relative
      % to each other starting from the roots.
      for task = graph.getRoots, task{1}.shiftDependentTasks(); end
    end

    function time = calculateDuration(graph)
      endTime = zeros(0);

      for task = graph.tasks, task = task{1};
        endTime(end + 1) = task.start + task.duration;
      end

      time = max(endTime);
    end

    function mobility = calculateMobility(graph)
      for task = graph.getLeaves, task{1}.propagateMobility(graph.deadline); end
    end
  end
end
