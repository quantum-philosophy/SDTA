classdef LS < handle
  properties
    graph
    priority
    schedule
  end

  methods
    function ls = LS(varargin)
      if nargin > 0, ls.process(varargin{:}); end
    end

    function schedule = process(ls, graph, priority)
      schedule = zeros(0, 0);

      if nargin < 3
        priority = zeros(0, 0);
        for i = 1:length(graph.orderedTasks)
          priority(i) = graph.orderedTasks{i}.deadline;
        end
      end

      ls.graph = graph;
      ls.priority = priority;

      taskIds = graph.getStartPoints();
      count = length(taskIds);
      done = zeros(1, length(priority));

      while count > 0
        % Find the most urgent task
        index = 1;
        for i = 2:count
          if priority(taskIds(index)) > priority(taskIds(i)), index = i; end
        end

        taskNo = taskIds(index);

        % Exclude the task
        taskIds(index) = [];
        count = count - 1;

        % Append to the schedule
        schedule = [ schedule taskNo ];

        % Append new tasks, ensure absence of repetitions
        newIds = graph.taskIndexesFrom{taskNo};
        for i = 1:length(newIds)
          if ~done(newIds(i))
            done(newIds(i)) = 1;
            count = count + 1;
            taskIds(count) = newIds(i);
          end
        end
      end

      ls.schedule = schedule;
    end

    function inspect(ls)
      if isempty(ls.graph)
        fprintf('Schedule is empty\n');
        return;
      end

      fprintf('Schedule for %s %d:\n', ls.graph.name, ls.graph.id);
      for i = 1:length(ls.schedule)
        task = ls.graph.orderedTasks{ls.schedule(i)};
        fprintf('  %s (%s)\n', task.name, num2str(ls.priority(i)));
      end
    end
  end
end
