classdef Graph < handle
  properties (SetAccess = private)
    name
    id
    tasks
    taskIndexesTo
    taskIndexesFrom
    taskTypes
    attributes
  end

  methods
    function graph = Graph(name, id)
      graph.name = name;
      graph.id = id;
      graph.tasks = {};
      graph.taskIndexesTo = {};
      graph.taskIndexesFrom = {};
      graph.taskTypes = zeros(0, 0);
      graph.attributes = containers.Map();
    end

    function taskIds = getStartPoints(graph)
      taskIds = zeros(0, 0);
      for i = 1:length(graph.tasks)
        if isempty(graph.taskIndexesTo{i}), taskIds(end + 1) = i; end
      end
    end

    function addTask(graph, name, type)
      task = TestCase.Task(name, type);

      graph.tasks{end + 1} = task;
      graph.taskTypes(end + 1) = type;

      index = length(graph.tasks);
      graph.taskIndexesTo{index} = zeros(0, 0);
      graph.taskIndexesFrom{index} = zeros(0, 0);
    end

    function addLink(graph, lname, fname, tname, type)
      ftask = graph.findTaskByName(fname);
      ttask = graph.findTaskByName(tname);

      link = TestCase.Link(lname, ftask, ttask, type);
      ftask.addOutLink(link);
      ttask.addInLink(link);

      % Save indexes of tasks
      findex = graph.findTaskIndexByName(fname);
      tindex = graph.findTaskIndexByName(tname);
      graph.taskIndexesTo{tindex} = [ graph.taskIndexesTo{tindex} findex ];
      graph.taskIndexesFrom{findex} = [ graph.taskIndexesFrom{findex} tindex ];
    end

    function addDeadline(graph, dname, tname, time)
      task = graph.findTaskByName(tname);
      if isa(time, 'char'), time = str2num(time); end
      task.deadline = time;
      graph.bubbleDeadline(task);
    end

    function setAttribute(graph, name, value)
      graph.attributes(name) = value;
    end

    function index = findTaskIndexByName(graph, name)
      for i = 1:length(graph.tasks)
        if strcmp(graph.tasks{i}.name, name)
          index = i;
          return;
        end
      end
      index = 0;
    end

    function task = findTaskByName(graph, name)
      index = graph.findTaskIndexByName(name);
      task = graph.tasks{index};
    end

    function inspect(graph)
      fprintf('Task graph: %s %d\n', graph.name, graph.id);
      fprintf('  Number of tasks: %d\n', length(graph.tasks));

      fprintf('  Attributes:\n');
      for key = graph.attributes.keys
        key = key{1};
        fprintf('    %s = %s\n', key, num2str(graph.attributes(key)));
      end

      fprintf('  Tasks:\n');
      for task = graph.tasks
        task = task{1};
        fprintf('    %s -> [ ', task.name);
        first = true;
        for link = task.outLinks
          if ~first
            fprintf(', ');
          else
            first = false;
          end
          fprintf('%s', link{1}.ttask.name);
        end
        fprintf(' ]\n');
      end
    end
  end

  methods (Access = private)
    function bubbleDeadline(graph, task)
      if isempty(task.inLinks), return; end
      time = task.deadline;
      for link = task.inLinks
        task = link{1}.ftask;
        if isempty(task.deadline) || (task.deadline > time)
          task.deadline = time;
          graph.bubbleDeadline(task);
        end
      end
    end
  end
end
