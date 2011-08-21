classdef LS < handle
  methods (Static)
    function schedule = schedule(graph, priority)
      if nargin < 2
        tcount = length(graph.tasks);
        priority = zeros(1, tcount);
        for i = 1:tcount, priority(i) = graph.tasks{i}.mobility; end
      end

      % Go to the ordinal numbers
      [ dummy, priority ] = sort(priority);

      taskCount = length(graph.tasks);

      % Obtain roots and sort them according to their priority
      ids = graph.getRootIds();
      [ dummy, I ] = sort(priority(ids));
      ids = ids(I);

      pool = graph.tasks(ids);

      processed = zeros(1, taskCount);
      scheduled = zeros(1, taskCount);

      processed(ids) = 1;

      schedule = zeros(0, 0);

      while ~isempty(pool)
        % The pool is always sorted according to the priority
        task = pool{1};

        % Exclude the task
        pool(1) = [];

        % Append to the schedule
        schedule(end + 1) = task.id;
        scheduled(task.id) = 1;

        % Append new tasks, but only ready ones, and ensure absence
        % of repetitions
        for child = task.children
          child = child{1};

          % Do not do again
          if processed(child.id), continue; end

          % All parents should be scheduled
          ready = true;
          for parent = child.parents
            parent = parent{1};
            if ~scheduled(parent.id)
              ready = false;
              break;
            end
          end

          % Is it ready or should we wait for another parent?
          if ~ready, continue; end

          % We need to insert it in the right place in order to keep
          % the pool sorted by priority
          index = 1;
          childPriority = priority(child.id);
          for competitor = pool
            competitor = competitor{1};
            if priority(competitor.id) > childPriority
              break;
            end
            index = index + 1;
          end
          if index > length(pool), pool{end + 1} = child;
          elseif index == 1, pool = { child pool{:} };
          else pool = { pool{1:index - 1} child pool{index:end} };
          end

          % We are done with it
          processed(child.id) = 1;
        end
      end

      graph.assignSchedule(schedule);
    end
  end
end
