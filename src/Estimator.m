classdef Estimator < handle
  methods (Static)
    function energy = calcEnergy(graph, pes, mapping)
      energy = 0;

      return;

      for i = 1:length(pes)
        pe = pes{i};
        mult = pe.attributes('voltage')^2 * pe.attributes('frequency');
        taskIds = find(mapping == i);
        taskTypes = graph.taskTypes(taskIds);

        power = pe.values(task.type);
      end
    end
  end
end
