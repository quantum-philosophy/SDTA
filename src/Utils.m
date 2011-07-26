classdef Utils
  methods (Static)
    function list = mapToVector(cells, property)
      list = zeros(0, 0);
      for cell = cells, list(end + 1) = cell{1}.(property); end
    end

    function result = include(cells, what)
      result = 0;
      if isa(what, 'char')
        % Compare strings
        for cell = cells
          if strcmp(cell{1}, what), result = 1; return; end
        end
      else
        % Compare as it is
        for cell = cells
          if cell{1} == what, result = 1; return; end
        end
      end
    end
  end
end
