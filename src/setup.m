function ssdtc = setup(name)
  if nargin < 1, name = 'simple'; end

  floorplan     = Utils.path([ name, '.flp' ]);
  config        = Utils.path('hotspot.config');
  testCase      = Utils.path([ name, '.tgff' ]);

  % Generate the test case
  Utils.startTimer('Generate a test case');
  if Utils.run('tgff', name, true) == 1, error('Cannot run TGFF'); end
  Utils.stopTimer();

  % Parse tasks graphs
  Utils.startTimer('Parse the test case');
  tgff = TestCase.TGFF(testCase);
  Utils.stopTimer();

  % Take just first graph for the moment
  graph = tgff.graphs{1};
  pes = tgff.pes;
  cores = length(pes);

  graph.inspect();
  for pe = pes, pe{1}.inspect(); end

  % Generate a floorplan
  Utils.startTimer('Generate a floorplan');
  Utils.generateFloorplan(floorplan, cores);
  Utils.stopTimer();

  % Thermal model
  thermalModel = HotSpot(floorplan, config);

  % Dummy mapping
  mapping = randi(cores, 1, graph.taskCount);

  Utils.inspectVector('Mapping', mapping);

  % LS scheduling
  Utils.startTimer('List scheduling');
  scheduler = Algorithms.LS(graph);
  Utils.stopTimer();

  scheduler.inspect();

  % Scheduling in time across the cores
  Utils.startTimer('Scheduling in time across all the cores');
  [ startTime, execTime ] = scheduler.calculateTime(pes, mapping);
  Utils.stopTimer();

  % Power profile
  Utils.startTimer('Generate a power profile');
  powerProfile = Power.calculateProfile(...
    graph, startTime, execTime, pes, mapping);
  Utils.stopTimer();

  steps = size(powerProfile, 1);

  fprintf('Number of steps: %d\n', steps);
  fprintf('Total simulation time: %.3f s\n', steps * Constants.samplingInterval);

  % Draw a bit
  Utils.drawSimulation(startTime, execTime, mapping, powerProfile);

  ssdtc = Algorithms.SSDTC(thermalModel, powerProfile);
end
