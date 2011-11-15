classdef Graph < handle
  properties (SetAccess = private)
    % General
    id
    name
    type

    % Timing
    hyperPeriod   % The period based on the LCM
    duration      % The duration of the graph without a slack
    deadline      % The deadline including a slack

    % Tasks
    tasks
    taskMap

    % PEs
    pes

    % Mapping
    isMapped
    mapping

    % Scheduling
    isScheduled
    schedule
    ordinalSchedule
    priority
  end

  methods
    function graph = Graph(id, name, type)
      % General
      graph.id = id;
      graph.name = name;
      graph.type = type;

      % Tasks
      graph.tasks = {};
      graph.taskMap = containers.Map();

      % Flags
      graph.isMapped = false;
      graph.isScheduled = false;
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

    function assignHyperPeriod(graph, value)
      graph.hyperPeriod = value;
    end

    function assignMapping(graph, pes, mapping)
      if ~isempty(pes), graph.pes = pes; end
      graph.mapping = mapping;
      graph.isMapped = true;

      % Calculate just durations of each task on its core
      graph.calculateTime();
    end

    function assignDeadline(graph, time)
      graph.deadline = time;
    end

    function tasks = getPETasks(graph, pe)
      if ~graph.isMapped, error('Should be mapped'); end

      ids = find(graph.mapping == pe.id);
      tasks = graph.tasks(ids);
    end

    function assignDistributedSchedule(graph, schedule)
      graph.schedule = [];
      graph.priority = [];
      graph.isScheduled = true;

      graph.ordinalSchedule = zeros(1, length(graph.tasks));

      for i = 1:length(schedule)
        localSchedule = schedule{i};
        for j = 1:size(localSchedule, 1)
          id = localSchedule(j, 1);
          start = localSchedule(j, 2);
          duration = localSchedule(j, 3);
          task = graph.tasks{id};

          graph.ordinalSchedule(id) = j;

          task.shift(start, duration);
        end
      end

      % Assign the actual duration
      graph.duration = graph.calculateDuration();
    end

    function assignSchedule(graph, schedule, priority)
      if ~graph.isMapped, error('Should be mapped'); end
      if nargin < 3, [ dummy, priority ] = sort(schedule); end

      % Order of the tasks, one id by another id
      graph.schedule = schedule;
      graph.priority = priority;
      graph.isScheduled = true;

      % Ordinal numbers of the tasks in the schedule
      [ dummy, graph.ordinalSchedule ] = sort(schedule);

      % Mapping is already there, distribute tasks in time!
      graph.distributeTime();

      % Assign the actual duration
      graph.duration = graph.calculateDuration();
    end

    function schedule = getPESchedule(graph, pe)
      if ~graph.isScheduled, error('Should be scheduled'); end

      ids = find(graph.mapping == pe.id);

      % Ordinal number of the tasks in the schedule
      ordinalSchedule = graph.ordinalSchedule(ids);

      % Sort tasks according to the schedule
      [ dummy, I ] = sort(ordinalSchedule);
      schedule = ids(I);
    end

    function fitTime(graph, desiredTime)
      graph.scale(desiredTime / graph.deadline);
    end

    function scale(graph, factor)
      if ~graph.isScheduled, error('Should be scheduled'); end

      for task = graph.tasks, task = task{1};
        task.scale(factor);
      end

      % Do not forget to adjust everything
      graph.duration = graph.duration * factor;
      graph.deadline = graph.deadline * factor;
    end

    function assign(graph, priority, mapping, schedule, start, duration)
      graph.priority = priority;

      graph.mapping = mapping;
      graph.isMapped = true;

      graph.schedule = schedule;
      graph.isScheduled = true;

      taskCount = length(graph.tasks);
      graph.ordinalSchedule = zeros(1, taskCount);

      for i = 1:taskCount
        graph.ordinalSchedule(schedule(i)) = i;
      end

      graph.duration = max(start + duration);

      for i = 1:taskCount
        graph.tasks{i}.assign(start(i), duration(i));
      end
    end

    function priority = averageMobility(graph, pes)
      graph.pes = pes;
      graph.calculateAverageTime();

      taskCount = length(graph.tasks);
      priority = zeros(1, taskCount);
      for i = 1:taskCount, priority(i) = graph.tasks{i}.mobility; end
    end
  end

  methods (Access = private)
    function calculateAverageTime(graph)
      processorCount = length(graph.pes);
      taskCount = length(graph.tasks);

      % Calculate execution times of the tasks
      for i = 1:taskCount
        total = 0;
        for j = 1:processorCount
          total = total + graph.pes{j}.calculateDuration(graph.tasks{i}.type);
        end
        graph.tasks{i}.assignDuration(total / processorCount);
      end

      % So, we know the average duration of the tasks,
      % we can compute their average ASAP and ALAP.
      graph.calculateASAP();
      graph.calculateALAP(); % and mobility
    end

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

      % So, we know the duration of the tasks,
      % we can compute their ASAP ans ALAP.
      graph.calculateASAP();
      graph.calculateALAP(); % and mobility
    end

    function distributeTime(graph)
      % First, put tasks in queues for each core
      for pe = graph.pes
        pe = pe{1};

        schedule = graph.getPESchedule(pe);

        ancestor = [];
        for id = schedule
          successor = graph.tasks{id};
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
      for task = graph.getRoots
        task{1}.propagateStartTime(0);
      end
    end

    function time = calculateDuration(graph)
      endTime = zeros(0);

      for task = graph.getLeaves
        task = task{1};
        endTime(end + 1) = task.start + task.duration;
      end

      time = max(endTime);
    end

    function time = calculateASAPDuration(graph)
      endTime = zeros(0);

      for task = graph.getLeaves
        task = task{1};
        endTime(end + 1) = task.asap + task.duration;
      end

      time = max(endTime);
    end

    function calculateASAP(graph)
      for task = graph.getRoots
        task{1}.propagateASAP(0);
      end
    end

    function calculateALAP(graph)
      if isempty(graph.deadline)
        duration = graph.calculateASAPDuration();
      else
        duration = graph.deadline;
      end
      for task = graph.getLeaves
        task{1}.propagateALAP(duration);
      end
    end
  end
end
