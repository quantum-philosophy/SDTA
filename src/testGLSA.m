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

vdd = zeros(0);
ngate = zeros(0);

for pe = graph.pes, pe = pe{1};
  vdd(end + 1) = pe.voltage;
  ngate(end + 1) = pe.ngate;
end

% First what we have without any affords
LS.schedule(graph); % Default priority of the tasks
figure;
Utils.drawMappingScheduling(graph);
dynamicPowerProfile = Power.calculateDynamicProfile(graph);

[ T, it ] = hotspot.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, vdd, ngate, ...
  GLSA.leakageTolerance, GLSA.maxLeakageIterations);

[ mttf1, cycles1 ] = Lifetime.predictAndDraw(T)

fprintf('MTTF without optimization is %f time units\n', min(mttf1));

% Now try to optimize with GLSA
glsa = GLSA();

Utils.startTimer('Solve with GLSA');
[ priority, fitness ] = glsa.solve(graph, hotspot);
Utils.stopTimer();

LS.schedule(graph, priority);
figure;
Utils.drawMappingScheduling(graph);
dynamicPowerProfile = Power.calculateDynamicProfile(graph);

[ T, it ] = hotspot.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, vdd, ngate, ...
  GLSA.leakageTolerance, GLSA.maxLeakageIterations);

[ mttf2, cycles2 ] = Lifetime.predictAndDraw(T)

fprintf('MTTF with optimization is %f time units\n', -fitness);

% Compare
fprintf('MTTF improvement is %.2f times\n', min(mttf2)/min(mttf1));
