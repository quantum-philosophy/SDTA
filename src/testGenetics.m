clear all;
clc;
rng(0);

name = 'test_cases/test_case_4_60';

testCase = Utils.path([ name, '.tgff' ]);
floorplan = Utils.path([ name, '.flp' ]);
config = Utils.path('hotspot.config');

tgff = TestCase.TGFF(testCase);
graph = tgff.graphs{1};
processors = tgff.pes;

mapping = Utils.generateEvenMapping(length(processors), length(graph.tasks));
graph.assignMapping(processors, mapping);

LS.schedule(graph);
graph.assignDeadline(Constants.deadlineFactor * graph.duration);

graph.inspect();

dynamicPowerProfile = Power.calculateDynamicProfile(graph);
[ steps, cores ] = size(dynamicPowerProfile);

fprintf('Steps: %d\n', steps);

pack = Utils.compactTaskGraph(graph, processors);

table = struct(...
  'ambient', Constants.ambientTemperature, ...
  'init_temp', Constants.ambientTemperature, ...
  'sampling_intvl', Constants.samplingInterval);

Genetics.optimizeAging(floorplan, config, table, pack.type - 1, pack.link, ...
  pack.frequency, pack.voltage, pack.ngate, pack.nc, pack.ceff);

hotspot = HotSpot(floorplan, config);

Utils.startTimer('Solve with the CE');
[ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage(...
  dynamicPowerProfile, pack.voltage, pack.ngate, 0.01, 10);
Utils.stopTimer();

fprintf('Iterations in Matlab: %d\n', it);

load('dynamic_power.mat');
load('temperature.mat');
load('total_power.mat');

T = T - Constants.degreeKelvin;
temperature = temperature - Constants.degreeKelvin;

figure;

x = ((1:steps) - 1) * Constants.samplingInterval;

subplot(3, 3, 1);
Utils.drawLines('Pdyn1', 'Time, s', 'P, W', x, dynamicPowerProfile);

subplot(3, 3, 2);
Utils.drawLines('Pdyn2', 'Time, s', 'P, W', x, dynamic_power);

subplot(3, 3, 3);
error = abs(dynamicPowerProfile - dynamic_power);
Utils.drawLines('dPdyn', 'Time, s', 'dP, W', x, error);

subplot(3, 3, 4);
Utils.drawLines('T1', 'Time, s', 'T, C', x, T);

subplot(3, 3, 5);
Utils.drawLines('T2', 'Time, s', 'T, C', x, temperature);

subplot(3, 3, 6);
error = abs(T - temperature);
Utils.drawLines('dT', 'Time, s', 'dT, C', x, error);

subplot(3, 3, 7);
Utils.drawLines('Ptot1', 'Time, s', 'P, W', x, totalPowerProfile);

subplot(3, 3, 8);
Utils.drawLines('Ptot2', 'Time, s', 'P, W', x, total_power);

subplot(3, 3, 9);
error = abs(totalPowerProfile - total_power);
Utils.drawLines('dPtot', 'Time, s', 'dP, W', x, error);
