classdef LS < handle
  methods (Static)
    function schedule = schedule(graph, priority)
      taskCount = length(graph.tasks);

      if nargin < 2 || isempty(priority)
        priority = zeros(1, taskCount);
        for i = 1:taskCount, priority(i) = graph.tasks{i}.mobility; end
      end

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

      graph.assignSchedule(schedule, priority);
    end

    function schedule = process(processors, graph, layout, priority)
      tasks = graph.tasks;

      taskCount = length(tasks);
      processorCount = length(processors);

      if nargin < 4 || isempty(priority)
        priority = zeros(1, taskCount);
        for i = 1:taskCount, priority(i) = graph.tasks{i}.mobility; end
      end

      processorTime = zeros(1, processorCount);
      taskTime = zeros(1, taskCount);

      % Obtain roots and sort them according to their priority
      ids = graph.getRootIds();
      [ dummy, I ] = sort(priority(ids));
      ids = ids(I);

      pool = cell(1, processorCount);

      processed = zeros(1, taskCount);
      scheduled = zeros(1, taskCount);

      for id = ids
        pid = layout(id);
        pool{pid} = [ pool{pid}, id ];
        processed(id) = 1;
      end

      schedule = cell(1, processorCount);

      while (true)
        empty = true;

        for pid = 1:processorCount
          if isempty(pool{pid}), continue; end
          empty = false;

          processorPool = pool{pid};
          id = processorPool(1);
          pool{pid} = processorPool(2:end);

          processor = processors{pid};
          task = tasks{id};

          start = max(processorTime(pid), taskTime(id));
          duration = processor.calculateDuration(task.type);
          finish = start + duration;

          processorTime(pid) = finish;

          schedule{pid} = [ schedule{pid}; id, start, duration ];
          scheduled(id) = true;

          % Append new tasks, but only ready ones, and ensure absence
          % of repetitions
          for child = task.children
            child = child{1};
            cid = child.id;

            taskTime(cid) = max(taskTime(cid), finish);

            % Do not do again
            if processed(cid), continue; end

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
            childPriority = priority(cid);
            childPool = pool{layout(cid)};
            for eid = childPool
              if priority(eid) > childPriority, break; end
              index = index + 1;
            end
            if index > length(childPool), childPool(end + 1) = cid;
            elseif index == 1, childPool = [ cid, childPool ];
            else childPool = [ childPool(1:index - 1), cid, childPool(index:end) ];
            end
            pool{layout(cid)} = childPool;

            % We are done with it
            processed(cid) = 1;
          end
        end

        if empty, break; end
      end
    end
  end
end
