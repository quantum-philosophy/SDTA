classdef Task < handle
  properties (SetAccess = private)
    % General
    id
    name
    type

    % Timing
    duration
    start     % Actual start time (mapped and scheduled)
    asap      % ASAP, as soon as possible
    alap      % ALAP, as late as possible
    mobility  % ALAP - ASAP

    % Dependencies in the graph
    isLeaf
    isRoot
    parents
    children

    % Dependencies on the same core
    ancestor
    successor
  end

  methods
    function task = Task(id, name, type)
      % General
      task.id = id;
      task.name = name;
      task.type = type;

      % Timing
      task.duration = 0;
      task.start = -Inf;
      task.asap = -Inf;
      task.alap = Inf;
      task.mobility = 0;

      % Dependencies in the graph
      task.isLeaf = true;
      task.isRoot = true;
      task.parents = {};
      task.children = {};

      % Dependencies on the same core
      task.ancestor = [];
      task.successor = [];
    end

    function resetMapping(task)
      task.start = -Inf;
      task.ancestor = [];
      task.successor = [];
    end

    function scale(task, factor)
      task.start = task.start * factor;
      task.duration = task.duration * factor;
    end

    function addParent(task, parent)
      task.parents{end + 1} = parent;
      task.isRoot = false;
    end

    function addChild(task, child)
      task.children{end + 1} = child;
      task.isLeaf = false;
    end

    function setAncestor(task, ancestor)
      task.ancestor = ancestor;
    end

    function setSuccessor(task, successor)
      task.successor = successor;
    end

    function assignDuration(task, time)
      task.duration = time;
    end

    function propagateStartTime(task, time)
      % We might already have an assigned start time with larger value
      if ~(task.start < time), return; end

      task.start = time;
      time = time + task.duration;

      % Shift data dependent tasks
      for child = task.children
        child{1}.propagateStartTime(time);
      end

      % Shift space dependent tasks (the same core)
      if ~isempty(task.successor)
        task.successor.propagateStartTime(time);
      end
    end

    function propagateASAP(task, asap)
      % We might already have an assigned ASAP with larger value
      if ~(task.asap < asap), return; end

      task.asap = asap;
      asap = asap + task.duration;

      % Shift data dependent tasks
      for child = task.children
        child{1}.propagateASAP(asap);
      end
    end

    function propagateALAP(task, time)
      % As late as possible
      alap = max(0, time - task.duration);

      % We might already have an assigned ALAP with smaller value
      if ~(alap < task.alap), return; end

      task.alap = alap;

      % Mobility = ALAP - ASAP
      task.mobility = max(0, alap - task.asap);

      % Shift data dependent tasks
      for parent = task.parents
        parent{1}.propagateALAP(alap);
      end
    end
  end
end
