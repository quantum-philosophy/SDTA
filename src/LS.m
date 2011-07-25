classdef LS < handle
  properties
    schedule
    graph
    priority
  end

  methods
    function ls = LS(graph)
      if nargin > 0
        ls.schedule(graph);
      end
    end

    function process(ls, graph, priority)
      ls.schedule = {};

      if nargin < 3
        priority = containers.Map();
        keys = graph.tasks.keys;
        for i = 1:length(keys)
          task = graph.tasks(keys{i});
          priority(keys{i}) = task.deadline;
        end
      end

      ls.graph = graph;
      ls.priority = priority;

      tasks = graph.getStartTasks();
      while ~isempty(tasks)
        % Find the most urgent task
        task = tasks{1};
        index = 1;
        for i = 2:length(tasks)
          if priority(task.name) > priority(tasks{i}.name)
            task = tasks{i};
            index = i;
          end
        end

        ls.schedule = { ls.schedule{:} task };

        % Exclude the task
        tasks(index) = [];

        % Append new tasks, ensure absence of repetitions
        if graph.links_from.isKey(task.name)
          links = graph.links_from(task.name);
          for i = 1:length(links)
            task = links{i}.ttask;
            if ~Utils.include(ls.schedule, task) && ~Utils.include(tasks, task)
              tasks = { tasks{:} task };
            end
          end
        end
      end
    end

    function inspect(ls)
      if isempty(ls.graph)
        fprintf('Schedule is empty\n');
        return;
      end

      fprintf('Schedule for %s %d:\n', ls.graph.name, ls.graph.id);
      for i = 1:length(ls.schedule)
        tname = ls.schedule{i}.name;
        fprintf('  %s (%s)\n', tname, num2str(ls.priority(tname)));
      end
    end
  end
end
