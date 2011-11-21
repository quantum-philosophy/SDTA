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

    function schedule = mapEarliestAndSchedule(graph, pes)
      tasks = graph.tasks;

      processorCount = length(pes);
      taskCount = length(tasks);

      priority = graph.averageMobility(pes);

      % Obtain roots and sort them according to their priority
      ids = graph.getRootIds();
      [ dummy, I ] = sort(priority(ids));
      ids = ids(I);

      pool = tasks(ids);

      processed = zeros(1, taskCount);
      scheduled = zeros(1, taskCount);

      processed(ids) = 1;

      schedule = zeros(0, 0);
      mapping = zeros(1, taskCount);
      start = zeros(1, taskCount);
      duration = zeros(1, taskCount);

      processorTime = zeros(1, processorCount);
      taskTime = zeros(1, taskCount);

      while ~isempty(pool)
        % The pool is always sorted according to the priority
        task = pool{1};

        % Exclude the task
        pool(1) = [];

        % Append to the schedule
        schedule(end + 1) = task.id;
        scheduled(task.id) = 1;

        % Find earliest processor
        pid = 1;
        earliestTime = processorTime(1);
        for i = 2:processorCount
          if processorTime(i) < earliestTime
            earliestTime = processorTime(i);
            pid = i;
          end
        end

        % Append to the mapping
        mapping(task.id) = pid;

        start(task.id) = max(taskTime(task.id), processorTime(pid));
        duration(task.id) = pes{pid}.calculateDuration(task.type);
        finish = start(task.id) + duration(task.id);

        processorTime(pid) = finish;

        % Append new tasks, but only ready ones, and ensure absence
        % of repetitions
        for child = task.children
          child = child{1};
          taskTime(child.id) = max(taskTime(child.id), finish);

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

      graph.assign(priority, mapping, schedule, start, duration);
    end

    function schedule = criticalityMapAndSchedule(graph, pes, hotspot)
      tasks = graph.tasks;

      processorCount = length(pes);
      taskCount = length(tasks);

      sc = graph.staticCriticality(pes);

      ids = graph.getRootIds();

      pool = tasks(ids);

      processed = zeros(1, taskCount);
      scheduled = zeros(1, taskCount);

      processed(ids) = 1;

      schedule = zeros(0, 0);
      mapping = zeros(1, taskCount);
      start = zeros(1, taskCount);
      duration = zeros(1, taskCount);

      processorTime = zeros(1, processorCount);
      taskTime = zeros(1, taskCount);
      energy = zeros(1, processorCount);

      time = zeros(taskCount, processorCount);
      power = zeros(taskCount, processorCount);

      for id = 1:taskCount
        for pid = 1:processorCount
          type = tasks{id}.type;
          time(id, pid) = pes{pid}.calculateDuration(type);
          power(id, pid) = pes{pid}.calculatePower(type);
        end
      end

      while ~isempty(pool)
        % Choose task and choose core
        decisionCount = length(pool);

        dc = ones(decisionCount, processorCount) * (-Inf);
        for i = 1:decisionCount
          id = pool{i}.id;
          for pid = 1:processorCount
            earliestTime = max([ taskTime(id), processorTime(pid) ]);
            t = earliestTime + time(id, pid);

            % 0.
            % dc(i, pid) = sc(id) - time(id, pid) - earliestTime;

            % 1.
            % e = energy(pid) + time(id, pid) * power(id, pid);
            % Pow = e / t;
            % dc(i, pid) = sc(id) - time(id, pid) - earliestTime - Pow;

            % 2.
            e = energy;
            e(pid) = e(pid) + time(id, pid) * power(id, pid);
            Temp = hotspot.steady(e / t);
            dc(i, pid) = sc(id) - time(id, pid) - earliestTime - max(Temp);
          end
        end

        I = 0;
        pid = 0;
        maxDU = -Inf;

        for i = 1:decisionCount
          for j = 1:processorCount
            if dc(i, j) > maxDU
              I = i;
              pid = j;
              maxDU = dc(i, j);
            end
          end
        end

        task = pool{I};
        id = task.id;

        % Exclude the task
        pool(I) = [];

        % Append to the schedule
        schedule(end + 1) = id;
        scheduled(id) = 1;

        % Append to the mapping
        mapping(id) = pid;

        start(id) = max(taskTime(id), processorTime(pid));
        duration(id) = time(id, pid);
        finish = start(id) + duration(id);

        processorTime(pid) = finish;

        energy(pid) = energy(pid) + duration(id) * power(id, pid);

        % Append new tasks, but only ready ones, and ensure absence
        % of repetitions
        for child = task.children
          child = child{1};
          taskTime(child.id) = max(taskTime(child.id), finish);

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

          pool{end + 1} = child;

          % We are done with it
          processed(child.id) = 1;
        end
      end

      graph.assign(schedule, mapping, schedule, start, duration);
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
