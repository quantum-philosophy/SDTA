function ssdtc = setup()
  clear all;
  clc;

  % Fix the randomness
  rng(0);

  floorplan     = Utils.path('simple.flp');
  graphConfig   = Utils.path('simple.tgff');
  hotspotConfig = Utils.path('hotspot.config');

  % Parse tasks graphs
  Utils.startTimer('Parse a test case');
  parser = TestCase.TGFFParser(graphConfig, ...
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
  ssdtc.inspect(true);
end
