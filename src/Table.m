classdef Table < handle
  properties
    name
    id
    attributes
    header
    values
  end

  methods
    function table = Table(name, id)
      table.name = name;
      table.id = id;
      table.attributes = containers.Map();
      table.header = {};
      table.values = zeros(0, 0);
    end

    function setAttributes(table, names, attributes)
      for i = 1:length(names)
        table.attributes(names{i}) = attributes(i);
      end
    end

    function setHeader(table, header)
      table.header = header;
    end

    function setRow(table, row, values)
      table.values(row, 1:length(values)) = values;
    end
  end
end
