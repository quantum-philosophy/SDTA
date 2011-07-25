classdef LS < handle
  properties
  end

  methods
    function ls = LS(graph)
      if nargin > 0
        ls.schedule(graph);
      end
    end

    function s = schedule(graph, priority)
      s = {};

      if nargin < 2
        priority = containers.Map();
        keys = graph.tasks.keys;
        for i = 1:length(keys)
          task = graph.tasks(keys(i));
          priority(keys(i)) = task.deadline;
        end
      end

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

        s = { s{:} task };

        % Exclude the task
        tasks(index) = [];

        % Append new tasks
        if graph.links_from.isKey(task.name)
          links = graph.links_from(task.name);
          for i = 1:length(links)
            tasks = { tasks{:} links{i}.ttask };
          end
        end
      end
    end
  end
end
