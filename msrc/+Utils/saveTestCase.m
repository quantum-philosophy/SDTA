function saveTestCase(graph, processors, filename)
  if nargin < 2, processors = graph.pes; end

  processorCount = length(processors);
  taskCount = length(graph.tasks);

  if nargin < 3
    filename = sprintf('test_cases/%03d_%03d', processorCount, taskCount);
    filename = Utils.path([ filename, '.config' ]);
  end

  systemConfig = Utils.compactTaskGraph(graph, processors);

  if isfield(systemConfig, 'type')
    systemConfig.type = systemConfig.type - 1;
  end

  if isfield(systemConfig, 'mapping')
    systemConfig.mapping = systemConfig.mapping - 1;
  end

  if isfield(systemConfig, 'schedule')
    systemConfig.schedule = systemConfig.schedule - 1;
  end

  Utils.dumpObject(systemConfig, filename);
end
