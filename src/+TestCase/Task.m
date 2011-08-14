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

    start
    duration
  end

  methods
    function task = Task(id, name, type)
      task.id = id;
      task.name = name;
      task.type = type;

      task.parents = {};
      task.children = {};

      task.resetMapping();
    end

    function resetMapping(task)
      task.ancestor = [];
      task.successor = [];

      task.start = 0;
      task.duration = 0;
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
  end
end
