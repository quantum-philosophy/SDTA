classdef Task < handle
  properties
    name
    type
    deadline
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
      task.inLinks = { task.inLinks{:} link };
    end

    function addOutLink(task, link)
      task.outLinks = { task.outLinks{:} link };
    end
  end
end
