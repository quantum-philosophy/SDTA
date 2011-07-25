classdef Link < handle
  properties
    name
    ftask
    ttask
    type
  end

  methods
    function link = Link(name, ftask, ttask, type)
      link.name = name;
      link.ftask = ftask;
      link.ttask = ttask;
      link.type = type;
    end
  end
end
