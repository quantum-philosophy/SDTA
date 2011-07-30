clear all;
clc;

% Fix the randomness
rng(0);

floorplan = '../build/simple.flp';
graphConfig = '../build/simple.tgff';
hotspotConfig = '../build/hotspot.config';
powerDump = '../build/simple.ptrace';

graphLabel = 'TASK_GRAPH';
peLabel = 'PE';

% Parse tasks graphs
Utils.startTimer('Parse a test case');
parser = TestCase.TGFFParser(graphConfig, { graphLabel }, { peLabel });
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

steps = ssdtc.stepCount;

x = ((1:steps) - 1) * Algorithms.TM.samplingInterval;

figure;

% The Condensed Equation Method
subplot(3, 1, 1);
[ T1, t1 ] = ssdtc.solveWithCondensedEquation();
Utils.drawLines(sprintf('Condensed Equation (%.3f s)', t1), ...
  'Time, s', 'Temperature, C', x, T1);

% Compare with HotSpot
subplot(3, 1, 2);
[ T2, t2 ] = ssdtc.solveWithHotSpot(2, 10);
Utils.drawLines(sprintf('HotSpot (%.3f s)', t2), ...
  'Time, s', 'Temperature, C', x, T2);

% Error
subplot(3, 1, 3);
error = T1 - T2;
Utils.drawLines(sprintf('Error T1 - T2 (max %.3f)', max(max(error))), ...
  'Time, s', 'Temperature, C', x, error);
