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

    function inspect(table)
      fprintf('Table: %s %d\n', table.name, table.id);
      fprintf('  Number of types: %d\n', length(table.values));

      fprintf('  Attributes:\n');
      keys = table.attributes.keys;
      for key = table.attributes.keys
        key = key{1};
        fprintf('    %s = %s\n', key, num2str(table.attributes(key)));
      end

      fprintf('  Types:\n');
      fprintf('%10s', 'id');
      for title = table.header
        fprintf('%15s', title{1});
      end
      fprintf('\n');
      for i = 1:size(table.values, 1)
        fprintf('%10d', i);
        for j = 1:size(table.values, 2)
          fprintf('%15s', num2str(table.values(i, j)));
        end
        fprintf('\n');
      end
    end
  end
end
