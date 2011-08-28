function pack = compactTaskGraph(graph, processors)
  tasks = graph.tasks;

  taskCount = length(tasks);
  processorCount = length(processors);

  pack = struct();

  type = zeros(1, taskCount);
  link = zeros(taskCount, taskCount);

  for i = 1:taskCount
    task = tasks{i};
    type(i) = task.type;
    for child = task.children
      link(i, child{1}.id) = 1;
    end
  end

  frequency = zeros(1, processorCount);
  voltage = zeros(1, processorCount);
  ngate = zeros(1, processorCount);
  nc = zeros(0, processorCount);
  ceff = zeros(0, processorCount);

  for i = 1:processorCount
    processor = processors{i};
    frequency(i) = processor.frequency;
    voltage(i) = processor.voltage;
    ngate(i) = processor.ngate;
    nc(1:length(processor.nc), i) = processor.nc;
    ceff(1:length(processor.ceff), i) = processor.ceff;
  end

  pack.type = type;
  pack.link = link;
  pack.frequency = frequency;
  pack.voltage = voltage;
  pack.ngate = ngate;
  pack.nc = nc;
  pack.ceff = ceff;
end
