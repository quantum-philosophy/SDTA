function ssdtc = setup(name)
  if nargin < 1, name = 'simple'; end

  floorplan     = Utils.path([ name, '.flp' ]);
  graphConfig   = Utils.path([ name, '.tgffopt' ]);
  testCase      = Utils.path([ name, '.tgff' ]);
  hotspotConfig = Utils.path('hotspot.config');

  % Generate the test case
  Utils.startTimer('Generate a test case');
  if Utils.run('tgff', name, true) == 1, error('Cannot run TGFF'); end
  Utils.stopTimer();

  % Parse tasks graphs
  Utils.startTimer('Parse the test case');
  parser = TestCase.TGFFParser(testCase, ...
    { Constants.graphLabel }, { Constants.peLabel });
  Utils.stopTimer();

  cores = length(parser.tables);

  % Generate a floorplan
  Utils.startTimer('Generate a floorplan');
  Utils.generateFloorplan(floorplan, cores);
  Utils.stopTimer();

  % Take just first graph for the moment
  graph = parser.graphs{1};

  ssdtc = Algorithms.SSDTC(graph, parser.tables, floorplan, hotspotConfig);
end
