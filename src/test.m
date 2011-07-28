clear all;
clc;

cores = 1;
floorplan = '../build/simple.flp';
tgffConfig = '../build/simple.tgff';
hotspotConfig = '../build/hotspot.config';
graphLabel = 'TASK_GRAPH';
peLabel = 'PE';
commLabel = 'COMMUN';

% Generate a floorplan
generateFloorplan(floorplan, cores);

% Parse the TGFF configuration file
tgff = TGFF(tgffConfig, { graphLabel }, { peLabel, commLabel });

pes = {};
comms = {};

for table = tgff.tables, table = table{1};
  if strcmp(table.name, peLabel), pes{end + 1} = table;
  elseif strcmp(table.name, commLabel), comms{end + 1} = table;
  end
end

colors = { 'r', 'g', 'b', 'm', 'y', 'c' };

% Steady-State Dynamic Temperature Curve for each task graph
for graph = tgff.graphs, graph = graph{1};
  ssdtc = SSDTC(graph, pes, comms, floorplan, hotspotConfig);
  x = 0:(ssdtc.stepCount - 1);

  figure;
  for i = 1:ssdtc.coreCount;
    color = colors{mod(i - 1, length(colors)) + 1};
    line(x, ssdtc.temperatureCurve(:, i), 'Color', color);
  end
end
