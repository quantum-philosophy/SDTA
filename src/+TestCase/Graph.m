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
    end
  end

  methods (Access = private)
    function bubbleDeadline(graph, tid, time)
      % Parents
      pids = graph.linksTo{tid};
      if isempty(pids), return; end
      for id = pids
        if graph.deadline(id) > time
          graph.deadline(id) = time;
          graph.bubbleDeadline(id, time);
        end
      end
    end
  end
end
