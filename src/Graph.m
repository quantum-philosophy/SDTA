classdef Graph < handle
  properties
    name
    id
    tasks
    taskIndexesTo
    taskIndexesFrom
    attributes
  end

  methods
    function graph = Graph(name, id)
      graph.name = name;
      graph.id = id;
      graph.tasks = {};
      graph.taskIndexesTo = {};
      graph.taskIndexesFrom = {};
      graph.attributes = containers.Map();
    end

    function taskIds = getStartPoints(graph)
      taskIds = zeros(0, 0);
      for i = 1:length(graph.tasks)
        if isempty(graph.taskIndexesTo{i}), taskIds = [ taskIds i ]; end
      end
    end

    function addTask(graph, name, type)
      task = Task(name, type);

      graph.tasks = { graph.tasks{:} task };

      index = length(graph.tasks);
      graph.taskIndexesTo{index} = zeros(0, 0);
      graph.taskIndexesFrom{index} = zeros(0, 0);
    end

    function addLink(graph, lname, fname, tname, type)
      ftask = graph.findTaskByName(fname);
      ttask = graph.findTaskByName(tname);

      link = Link(lname, ftask, ttask, type);
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

    function bubbleDeadline(graph, task)
      if isempty(task.inLinks), return; end
      time = task.deadline;
      links = task.inLinks;
      for i = 1:length(links)
        task = links{i}.ftask;
        if isempty(task.deadline) || (task.deadline > time)
          task.deadline = time;
          graph.bubbleDeadline(task);
        end
      end
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
      keys = graph.attributes.keys;
      for i = 1:length(keys)
        fprintf('    %s = %s\n', keys{i}, num2str(graph.attributes(keys{i})));
      end

      fprintf('  Tasks:\n');
      for i = 1:length(graph.tasks)
        task = graph.tasks{i};
        fprintf('    %s -> [ ', task.name);
        links = task.outLinks;
        for j = 1:length(links)
          if j > 1, fprintf(', '); end
          fprintf('%s', links{j}.ttask.name);
        end
        fprintf(' ]\n');
      end
    end
  end
end
