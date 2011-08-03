classdef LS < handle
  properties (SetAccess = private)
    graph
    priority
    schedule
  end

  methods
    function ls = LS(graph, priority)
      if nargin < 3
        priority = zeros(0, 0);
        for id = 1:graph.taskCount
          priority(end + 1) = graph.deadline(id);
        end
      end

      tasks = graph.taskCount;

      pool = graph.getStartPoints();
      done = zeros(1, tasks);
      done(pool) = 1;

      schedule = zeros(0, 0);

      while ~isempty(pool)
        % Find the most urgent task (the lower priority, more urgent)
        [ dummy, index ] = min(priority(pool));
        id = pool(index);

        % Exclude the task
        pool(index) = [];

        % Append to the schedule
        schedule(end + 1) = id;

        % Append new tasks, but only ready ones, and ensure absence
        % of repetitions
        nids = graph.linksFrom{id};
        for nid = nids
          if done(nid), continue; end
          ins = graph.linksTo{nid};
          nin = length(ins);
          if nin == 1 || nin == length(intersect(ins, find(done)))
            done(nid) = 1;
            pool(end + 1) = nid;
          end
        end
      end

      ls.graph = graph;
      ls.priority = priority;
      ls.schedule = schedule;
    end

    function [ startTime, execTime ] = calculateTime(ls, pes, mapping)
      cores = length(pes);
      tasks = ls.graph.taskCount;

      execTime = zeros(1, tasks);
      startTime = zeros(1, tasks);

      coreSchedule = cell(cores);

      for i = 1:cores
        coreSchedule{i} = zeros(0, 0);

        ids = find(mapping == i);
        if isempty(ids), continue; end

        types = ls.graph.taskTypes(ids);

        % t = NC / f
        execTime(ids) = pes{i}.nc(types) / pes{i}.frequency;

        % Calculate a local schedule, shift its tasks relative to each other
        shift = 0;
        for id = ls.schedule
          if any(ids == id)
            coreSchedule{i}(end + 1) = id;
            startTime(id) = shift;
            shift = shift + execTime(id);
          end
        end
      end

      % Now consider dependencies between tasks
      pool = ls.schedule;
      inpool = ones(1, tasks);
      while ~isempty(pool)
        id = pool(1);
        pool(1) = [];
        inpool(id) = 0;

        finish = startTime(id) + execTime(id);

        nids = ls.graph.linksFrom{id};
        for nid = nids
          shift = finish - startTime(nid);
          if shift < 0, continue; end

          % Shift the core schedule
          ncid = mapping(nid);
          found = 0;
          for sid = coreSchedule{ncid}
            if ~found && sid == nid, found = 1; end
            if found
              startTime(sid) = startTime(sid) + shift;
              if ~inpool(sid)
                % We need to consider it once again
                pool(end + 1) = sid;
                inpool(sid) = 1;
              end
            end
          end
        end
      end
    end

    function inspect(ls)
      Utils.inspectVector('Schedule', ls.schedule);
    end
  end
end
