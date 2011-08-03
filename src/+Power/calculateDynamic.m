function taskPower = calculateDynamic(graph, pes, mapping)
  cores = length(pes);
  tasks = graph.taskCount;

  taskPower = zeros(1, tasks);

  for i = 1:cores
    pe = pes{i};

    ids = find(mapping == i);
    if isempty(ids), continue; end

    types = graph.taskTypes(ids);

    % Pdyn = Ceff * f * Vdd^2
    taskPower(ids) = pe.ceff(types) * pe.frequency * pe.voltage^2;
  end
end

