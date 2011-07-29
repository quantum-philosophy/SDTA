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
Utils.generateFloorplan(floorplan, cores);

% Parse tasks graphs
parser = TestSet.TGFFParser(graphConfig, { graphLabel }, { peLabel });

colors = { 'r', 'g', 'b', 'm', 'y', 'c' };

rng(0);

% Steady-State Dynamic Temperature Curve for each task graph
for graph = parser.graphs, graph = graph{1};
  ssdtc = Algorithms.SSDTC(graph, parser.tables, floorplan, hotspotConfig);
  ssdtc.inspect();

  return;

  tic
  T = ssdtc.solveWithCondensedEquation();
  toc
  tic
  T1 = ssdtc.solveWithHotSpot(2, 10);
  toc

  % Plotting
  figure;
  x = 0:(size(T, 1) - 1);
  for i = 1:size(T, 2)
    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, T(:, i), 'Color', color);
    line(x, T1(:, i), 'Color', color, 'LineStyle', '--');
  end
end
