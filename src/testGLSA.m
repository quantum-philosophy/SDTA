% Test: Testdrive for the genetic algorithm for scheduling

clear all;
clc;
rng(0);

name = 'test_cases/test_case_4_60';

floorplan = Utils.path([ name, '.flp' ]);
testCase = Utils.path([ name, '.tgff' ]);
config = Utils.path('hotspot.config');

tgff = TestCase.TGFF(testCase);
graph = tgff.graphs{1};
pes = tgff.pes;

hotspot = HotSpot(floorplan, config);

mapping = Utils.generateEvenMapping(length(pes), length(graph.tasks));
graph.assignMapping(pes, mapping);

graph.inspect();

glsa = GLSA();

Utils.startTimer('Solve with GLSA');
[ priority, fitness ] = glsa.solve(graph, hotspot);
Utils.stopTimer();

LS.schedule(graph, priority);

Utils.drawMappingScheduling(graph);

dynamicPowerProfile = Power.calculateDynamicProfile(graph);

[ T, it ] = glsa.thermalModel.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, glsa.vdd, glsa.ngate, ...
  glsa.leakageTolerance, glsa.maxLeakageIterations);

mttf = Lifetime.predictAndDraw(T)

fprintf('MTTF = %f\n', -fitness);
