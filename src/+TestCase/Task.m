classdef Task < handle
  properties (SetAccess = private)
    id
    name
    type
    deadline

    % In the graph
    parents
    children

    % On the same core
    ancestor
    successor

    duration

    start     % ASAP, as soon as possible
    alap      % ALAP, as late as possible
    mobility  % ALAP - ASAP
  end

  methods
    function task = Task(id, name, type)
      task.id = id;
      task.name = name;
      task.type = type;

      task.parents = {};
      task.children = {};

      task.duration = 0;

      task.resetMapping();
    end

    function scale(task, factor)
      task.start = task.start * factor;
      task.duration = task.duration * factor;
    end

    function resetMapping(task)
      task.ancestor = [];
      task.successor = [];
      task.start = 0;
      task.alap = Inf;
      task.mobility = 0;
    end

    function addParent(task, parent)
      task.parents{end + 1} = parent;
    end

    function addChild(task, child)
      task.children{end + 1} = child;
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

    function assignDeadline(task, time)
      if ~isempty(task.deadline) && task.deadline <= time, return; end
      task.deadline = time;
      for parent = task.parents
        parent{1}.assignDeadline(max(0, time - 1));
      end
    end

    function assignStartTime(task, time)
      task.start = time;
    end

    function result = isRoot(task)
      result = isempty(task.parents);
    end

    function result = isLeaf(task)
      result = isempty(task.children);
    end

    function shiftDependentTasks(task, time)
      if nargin > 1
        if task.start >= time, return; end
      else
        time = 0;
      end

      task.start = time;
      finish = time + task.duration;

      % Shift data dependent tasks
      for child = task.children
        child{1}.shiftDependentTasks(finish);
      end

      % Shift space dependent tasks (the same core)
      if ~isempty(task.successor)
        task.successor.shiftDependentTasks(finish);
      end
    end

    function propagateMobility(task, time)
      % As late as possible
      alap = time - task.duration;

      % We might already have an assigned mobility with
      % smaller ALAP time
      if ~(alap < task.alap), return; end

      task.alap = alap;

      % Mobility = ALAP - ASAP
      task.mobility = alap - task.start;

      % Shift data dependent tasks
      for parent = task.parents
        parent{1}.propagateMobility(alap);
      end

      % Shift space dependent tasks (the same core)
      if ~isempty(task.ancestor)
        task.ancestor.propagateMobility(alap);
      end
    end
  end
end
