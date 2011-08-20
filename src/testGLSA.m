% Test: Testdrive for the genetic algorithm for scheduling

clear all;
clc;
rng(3);

[ graph, hotspot, powerProfile ] = setup('test_cases/test_case_4_60');

vdd = zeros(0);
ngate = zeros(0);

for pe = graph.pes, pe = pe{1};
  vdd(end + 1) = pe.voltage;
  ngate(end + 1) = pe.ngate;
end

% First, without any efforts
[ T, it ] = hotspot.solveCondensedEquationWithLeakage( ...
  powerProfile, vdd, ngate, GLSA.leakageTolerance, GLSA.maxLeakageIterations);
[ mttf1, cycles1 ] = Lifetime.predictAndDraw(T);
fprintf('MTTF without optimization is %.2f time units\n', min(mttf1));

% Now try to optimize with GLSA
glsa = GLSA();
Utils.startTimer('Solve with GLSA');
[ priority, fitness, output ] = glsa.solve(graph, hotspot, true);
Utils.stopTimer();
fprintf('Number of generation is %d\n', output.generations);

% Calculate the best one
LS.schedule(graph, priority);
powerProfile = Power.calculateDynamicProfile(graph);
[ T, it ] = hotspot.solveCondensedEquationWithLeakage( ...
  powerProfile, vdd, ngate, GLSA.leakageTolerance, GLSA.maxLeakageIterations);
[ mttf2, cycles2 ] = Lifetime.predictAndDraw(T);
fprintf('MTTF with optimization is %.2f time units\n', -fitness);

% Compare
fprintf('MTTF improved by %.2f times\n', min(mttf2)/min(mttf1));
