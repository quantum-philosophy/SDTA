function [ thermalModel, powerProfile, cores, steps ] = setup(name, debug, draw)
  if nargin < 1, name = 'simple'; end
  if nargin < 2, debug = true; end
  if nargin < 3, draw = true; end
  if ~debug, draw = false; end

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
  thermalModel = HotSpot(floorplan, config);

  % Dummy mapping
  mapping = Utils.generateEvenMapping(cores,length(graph.tasks));
  graph.assignMapping(pes, mapping);

  % LS scheduling
  Utils.startTimer('Scheduling in time across all the cores');
  LS.schedule(graph);
  Utils.stopTimer();

  if debug, graph.inspect(); end

  % Power profile
  Utils.startTimer('Generate a dynamic power profile');
  powerProfile = Power.calculateDynamicProfile(graph);
  Utils.stopTimer();

  steps = size(powerProfile, 1);

  if debug
    fprintf('Number of steps: %d\n', steps);
    fprintf('Total simulation time: %.3f s\n', steps * Constants.samplingInterval);
  end

  if draw
    % Draw a bit
    Utils.drawSimulation(graph, powerProfile);
  end
end
