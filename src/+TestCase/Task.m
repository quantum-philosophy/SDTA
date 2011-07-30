classdef Task < handle
  properties
    deadline
  end

  properties (SetAccess = private)
    name
    type
    inLinks
    outLinks
  end

  methods
    function task = Task(name, type)
      task.name = name;
      task.type = type;
      task.inLinks = {};
      task.outLinks = {};
    end

    function addInLink(task, link)
      task.inLinks{end + 1} = link;
    end

    function addOutLink(task, link)
      task.outLinks{end + 1} = link;
    end
  end
end
