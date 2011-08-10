classdef Graph < handle
  properties (SetAccess = private)
    name
    id

    period

    taskCount

    nameMap

    linksTo
    linksFrom
    taskTypes

    deadline

    pes
    mapping
    schedule
    startTime
    execTime
  end

  methods
    function graph = Graph(name, id)
      graph.name = name;
      graph.id = id;

      graph.taskCount = 0;

      graph.nameMap = containers.Map();

      graph.linksTo = {};
      graph.linksFrom = {};
      graph.taskTypes = zeros(0, 0);

      graph.deadline = zeros(0, 0);
    end

    function ids = getStartPoints(graph)
      ids = zeros(0, 0);
      for id = 1:graph.taskCount
        if isempty(graph.linksTo{id})
          ids(end + 1) = id;
        end
      end
    end

    function addTask(graph, name, type)
      graph.taskTypes(end + 1) = type;
      graph.linksTo{end + 1} = zeros(0, 0);
      graph.linksFrom{end + 1} = zeros(0, 0);
      graph.taskCount = graph.taskCount + 1;
      graph.nameMap(name) = graph.taskCount;
    end

    function addLink(graph, lname, fname, tname, type)
      fid = graph.nameMap(fname);
      tid = graph.nameMap(tname);
      graph.linksTo{tid} = [ graph.linksTo{tid} fid ];
      graph.linksFrom{fid} = [ graph.linksFrom{fid} tid ];
    end

    function setPeriod(graph, value)
      graph.period = value;
    end

    function setDeadline(graph, dname, tname, time)
      id = graph.nameMap(tname);
      graph.deadline(id) = time;
      graph.bubbleDeadline(id, time);
    end

    function inspect(graph)
      fprintf('Task graph: %s %d\n', graph.name, graph.id);
      fprintf('  Period: %f\n', graph.period);
      fprintf('  Number of tasks: %d\n', graph.taskCount);
      fprintf('  Tasks:\n');

      for id = 1:graph.taskCount
        fprintf('    %d -> [ ', id);
        first = true;
        for lid = graph.linksFrom{id}
          if ~first, fprintf(', ');
          else first = false;
          end
          fprintf('%d', lid);
        end
        fprintf(' ]\n');
      end

      for pe = graph.pes, pe{1}.inspect(); end

      Utils.inspectVector('Mapping', graph.mapping);
      Utils.inspectVector('Schedule', graph.schedule);
    end

    function assignMapping(graph, pes, mapping)
      graph.pes = pes;
      graph.mapping = mapping;
    end

    function assignSchedule(graph, schedule)
      graph.schedule = schedule;
      graph.calculateTime();
    end
  end

  methods (Access = private)
    function bubbleDeadline(graph, tid, time)
      % Parents
      pids = graph.linksTo{tid};
      if isempty(pids), return; end
      for id = pids
        if length(graph.deadline) < id || graph.deadline(id) > time
          graph.deadline(id) = time;
          graph.bubbleDeadline(id, time);
        end
      end
    end

    function calculateTime(graph)
      pes = graph.pes;
      mapping = graph.mapping;

      cores = length(pes);
      tasks = graph.taskCount;

      execTime = zeros(1, tasks);
      startTime = zeros(1, tasks);

      coreSchedule = cell(cores);

      for i = 1:cores
        coreSchedule{i} = zeros(0, 0);

        ids = find(mapping == i);
        if isempty(ids), continue; end

        types = graph.taskTypes(ids);

        % t = NC / f
        execTime(ids) = pes{i}.nc(types) / pes{i}.frequency;

        % Calculate a local schedule, shift its tasks relative to each other
        shift = 0;
        for id = graph.schedule
          if any(ids == id)
            coreSchedule{i}(end + 1) = id;
            startTime(id) = shift;
            shift = shift + execTime(id);
          end
        end
      end

      % Now consider dependencies between tasks
      pool = graph.schedule;
      inpool = ones(1, tasks);
      while ~isempty(pool)
        id = pool(1);
        pool(1) = [];
        inpool(id) = 0;

        finish = startTime(id) + execTime(id);

        nids = graph.linksFrom{id};
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

      graph.startTime = startTime;
      graph.execTime = execTime;
    end
  end
end
