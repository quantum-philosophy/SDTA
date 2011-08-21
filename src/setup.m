function [ graph, hotspot, powerProfile ] = setup(name, debug)
  if nargin < 1, name = 'simple'; end
  if nargin < 2, debug = true; end

  % NOTE: Should already be generated!
  floorplan = Utils.path([ name, '.flp' ]);
  testCase = Utils.path([ name, '.tgff' ]);
  config = Utils.path('hotspot.config');

  % Parse tasks graphs
  Utils.startTimer('Parse the test case');
  tgff = TestCase.TGFF(testCase);
  Utils.stopTimer();

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
  Utils.startTimer('Scheduling in time across all the cores');
  LS.schedule(graph);
  Utils.stopTimer();

  graph.assignDeadline(Constants.deadlineFactor * graph.duration);

  if debug, graph.inspect(); end

  if nargout < 3, return; end

  % Power profile
  Utils.startTimer('Generate a dynamic power profile');
  powerProfile = Power.calculateDynamicProfile(graph);
  Utils.stopTimer();

  if debug
    steps = size(powerProfile, 1);
    fprintf('Number of steps: %d\n', steps);
  end
end
