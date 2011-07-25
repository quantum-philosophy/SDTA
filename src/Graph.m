classdef Graph < handle
  properties
    name
    id
    tasks
    links_to
    links_from
    attributes
  end

  methods
    function graph = Graph(name, id)
      graph.name = name;
      graph.id = id;
      graph.tasks = containers.Map();
      graph.links_to = containers.Map();
      graph.links_from = containers.Map();
      graph.attributes = containers.Map();
    end

    function tasks = getStartTasks(graph)
      tasks = {};
      keys = graph.tasks.keys;
      for i = 1:length(keys)
        if ~graph.links_to.isKey(keys(i))
          tasks = { tasks{:} graph.tasks(keys{i}) };
        end
      end
    end

    function addTask(graph, name, type)
      graph.tasks(name) = Task(name, type);
    end

    function addLink(graph, lname, fname, tname, type)
      ftask = graph.tasks(fname);
      ttask = graph.tasks(tname);
      link = Link(lname, ftask, ttask, type);
      Utils.addToList(graph.links_from, fname, link);
      Utils.addToList(graph.links_to, tname, link);
    end

    function addDeadline(graph, dname, tname, time)
      task = graph.tasks(tname);
      if isa(time, 'char'), time = str2num(time); end
      task.deadline = time;
      graph.bubbleDeadline(task);
    end

    function bubbleDeadline(graph, task)
      if ~graph.links_to.isKey(task.name), return; end
      time = task.deadline;
      links = graph.links_to(task.name);
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
  end
end
