classdef Utils
  methods (Static)
    function addToList(map, key, value)
      if map.isKey(key)
        old = map(key);
        map(key) = { old{:} value };
      else
        map(key) = { value };
      end
    end

    function list = map(cells, property)
      list = {};
      for i = 1:length(cells)
        list = { list{:} cells{i}.(property) };
      end
    end

    function result = include(cells, what)
      result = 0;
      if isa(what, 'char')
        % Compare strings
        for i = 1:length(cells)
          if strcmp(cells{i}, what), result = 1; return; end
        end
      else
        % Compare as it is
        for i = 1:length(cells)
          if cells{i} == what, result = 1; return; end
        end
      end
    end
  end
end
