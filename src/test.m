clear all;
clc;

floorplan = '../build/simple.flp';
graphConfig = '../build/simple.tgff';
hotspotConfig = '../build/hotspot.config';
powerDump = '../build/simple.ptrace';

cores = 1;
graphLabel = 'TASK_GRAPH';
peLabel = 'PE';

% Generate a floorplan
Utils.startTimer('Generate a floorplan');
Utils.generateFloorplan(floorplan, cores);
Utils.stopTimer();

% Parse tasks graphs
Utils.startTimer('Parse a test case');
parser = TestSet.TGFFParser(graphConfig, { graphLabel }, { peLabel });
Utils.stopTimer();

colors = { 'r', 'g', 'b', 'm', 'y', 'c' };

rng(0);

% Steady-State Dynamic Temperature Curve for each task graph
for graph = parser.graphs, graph = graph{1};
  ssdtc = Algorithms.SSDTC(graph, parser.tables, floorplan, hotspotConfig);

  T = ssdtc.solveWithCondensedEquation();
  % T = ssdtc.solveWithHotSpot(2, 10);

  ssdtc.inspect(true);
end
