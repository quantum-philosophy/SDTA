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
  end
end
