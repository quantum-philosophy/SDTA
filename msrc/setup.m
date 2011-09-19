function [ graph, hotspot, powerProfile ] = setup(name)
  if nargin < 1, name = 'simple'; end

  % NOTE: Should already be generated!
  floorplan = Utils.path([ name, '.flp' ]);
  testCase = Utils.path([ name, '.tgff' ]);
  config = Utils.path('hotspot.config');

  % Parse tasks graphs
  tgff = TestCase.TGFF(testCase);

  % Take just first graph for the moment
  graph = tgff.graphs{1};
  pes = tgff.pes;
  cores = length(pes);

  % Thermal model
  hotspot = HotSpot(floorplan, config);

  % Dummy mapping
  mapping = Utils.generateEvenMapping(cores, length(graph.tasks));
  graph.assignMapping(pes, mapping);

  % LS scheduling
  schedule = LS.process(pes, graph, mapping);
  graph.assignDistributedSchedule(schedule);

  graph.assignDeadline(Constants.deadlineFactor * graph.duration);

  if nargout < 3, return; end

  % Power profile
  powerProfile = Power.calculateDynamicProfile(graph);
end
