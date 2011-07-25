classdef Graph < handle
  properties
    name
    id
    tasks
    orderedTasks
    linksTo
    linksFrom
    taskIndexesTo
    taskIndexesFrom
    attributes
  end

  methods
    function graph = Graph(name, id)
      graph.name = name;
      graph.id = id;

      graph.tasks = containers.Map();
      graph.orderedTasks = {};

      % To search by names of tasks
      graph.linksTo = containers.Map();
      graph.linksFrom = containers.Map();

      % To search by indexes of tasks
      graph.taskIndexesTo = {};
      graph.taskIndexesFrom = {};

      graph.attributes = containers.Map();
    end

    function taskIds = getStartPoints(graph)
      taskIds = zeros(0, 0);
      for i = 1:length(graph.orderedTasks)
        if isempty(graph.taskIndexesTo{i}), taskIds = [ taskIds i ]; end
      end
    end

    function addTask(graph, name, type)
      task = Task(name, type);

      graph.tasks(name) = task;
      graph.orderedTasks = { graph.orderedTasks{:} task };

      % Keep track of Link objects
      graph.linksTo(name) = {};
      graph.linksFrom(name) = {};

      % Simplified tracking or task indexes
      index = length(graph.orderedTasks);
      graph.taskIndexesTo{index} = zeros(0, 0);
      graph.taskIndexesFrom{index} = zeros(0, 0);
    end

    function addLink(graph, lname, fname, tname, type)
      ftask = graph.tasks(fname);
      ttask = graph.tasks(tname);

      link = Link(lname, ftask, ttask, type);

      % Save Link objects
      links = graph.linksTo(tname);
      graph.linksTo(tname) = { links{:} link };
      links = graph.linksFrom(fname);
      graph.linksFrom(fname) = { links{:} link };

      % Save indexes of tasks
      findex = graph.findTaskIndexByName(fname);
      tindex = graph.findTaskIndexByName(tname);
      graph.taskIndexesTo{tindex} = [ graph.taskIndexesTo{tindex} findex ];
      graph.taskIndexesFrom{findex} = [ graph.taskIndexesFrom{findex} tindex ];
    end

    function addDeadline(graph, dname, tname, time)
      task = graph.tasks(tname);
      if isa(time, 'char'), time = str2num(time); end
      task.deadline = time;
      graph.bubbleDeadline(task);
    end

    function bubbleDeadline(graph, task)
      if ~graph.linksTo.isKey(task.name), return; end
      time = task.deadline;
      links = graph.linksTo(task.name);
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
      for i = 1:length(graph.orderedTasks)
        if strcmp(graph.orderedTasks{i}.name, name)
          index = i;
          return;
        end
      end
      index = 0;
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
      keys = graph.tasks.keys;
      for i = 1:length(keys)
        task = graph.tasks(keys{i});
        fprintf('    %s -> [ ', task.name);
        if graph.linksFrom.isKey(task.name)
          links = graph.linksFrom(task.name);
          for j = 1:length(links)
            if j > 1, fprintf(', '); end
            fprintf('%s', links{j}.ttask.name);
          end
        end
        fprintf(' ]\n');
      end
    end
  end
end
