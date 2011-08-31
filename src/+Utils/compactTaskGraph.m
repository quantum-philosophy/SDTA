function systemConfig = compactTaskGraph(graph, processors)
  tasks = graph.tasks;

  taskCount = length(tasks);
  processorCount = length(processors);

  type = zeros(1, taskCount);
  link = zeros(taskCount, taskCount);

  for i = 1:taskCount
    task = tasks{i};
    type(i) = task.type;
    for child = task.children
      link(i, child{1}.id) = 1;
    end
  end

  type = uint32(type);
  link = logical(link);

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

  frequency = uint32(frequency);
  ngate = uint32(ngate);
  nc = uint32(nc);

  systemConfig = struct('type', type, 'link', link, 'frequency', frequency, ...
    'voltage', voltage, 'ngate', ngate, 'nc', nc, 'ceff', ceff);
end
