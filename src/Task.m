classdef Task < handle
  properties
    name
    type
    deadline
  end

  methods
    function task = Task(name, type)
      task.name = name;
      task.type = type;
    end
  end
end
