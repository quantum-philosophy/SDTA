clear all;
clc;

floorplan = '../build/simple.flp';
graphConfig = '../build/simple.tgff';
hotspotConfig = '../build/hotspot.config';
powerDump = '../build/simple.ptrace';

cores = 1;
graphLabel = 'TASK_GRAPH';
peLabel = 'PE';
commLabel = 'COMMUN';

% Generate a floorplan
Utils.generateFloorplan(floorplan, cores);

% Parse tasks graphs
parser = TestSet.TGFFParser(graphConfig, { graphLabel }, { peLabel, commLabel });

pes = {};
comms = {};

for table = parser.tables, table = table{1};
  if strcmp(table.name, peLabel), pes{end + 1} = table;
  elseif strcmp(table.name, commLabel), comms{end + 1} = table;
  end
end

colors = { 'r', 'g', 'b', 'm', 'y', 'c' };

% Steady-State Dynamic Temperature Curve for each task graph
for graph = parser.graphs, graph = graph{1};
  ssdtc = Algorithms.SSDTC(graph, pes, comms, floorplan, hotspotConfig);

  tic
  T = ssdtc.solveWithCondensedEquation();
  % T = ssdtc.solveWithHotSpot();
  toc

  % Plotting
  figure;
  x = 0:(size(T, 1) - 1);
  for i = 1:size(T, 2)
    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, T(:, i), 'Color', color);
  end
end
