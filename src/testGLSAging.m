% Test: Testdrive for the genetic algorithm for scheduling

clear all;
clc;
rng(0);

[ graph, hotspot, dynamicPowerProfile ] = setup('test_cases/test_case_4_60');

vdd = zeros(0);
ngate = zeros(0);

for pe = graph.pes, pe = pe{1};
  vdd(end + 1) = pe.voltage;
  ngate(end + 1) = pe.ngate;
end

tuning = Genetic.LSAging.defaultTuning( ...
  'generationStall', 20, ...
  'generationTolerance', 0.01, ...
  'generationLimit', 100, ...
  'mobilityCreationFactor', 0.5, ...
  'populationSize', 50, ...
  'generationalGap', 0.5, ...
  'crossoverFraction', 0.8, ...
  'minimalMutationProbability', 0.1 ...
);

% First, without any efforts
[ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, vdd, ngate, tuning.leakageTolerance, ...
  tuning.maxLeakageIterations);
[ mttf1, cycles1 ] = Lifetime.predict(T);
fprintf('MTTF without optimization: %.2f\n', min(mttf1));
fprintf('Energy: %.2f J\n', sum(sum(totalPowerProfile * Constants.samplingInterval)));

drawing = figure;

% Now try to optimize with GLSA
ls = Genetic.LSAging(graph, hotspot, tuning);
Utils.startTimer('Solve with the GLSA');
[ priority, fitness, output ] = ls.solve(drawing);
Utils.stopTimer();
fprintf('Number of generation: %d\n', output.generations);

% Calculate the best one
LS.schedule(graph, priority);
dynamicPowerProfile = Power.calculateDynamicProfile(graph);
[ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, vdd, ngate, tuning.leakageTolerance, ...
  tuning.maxLeakageIterations);
[ mttf2, cycles2 ] = Lifetime.predict(T);
fprintf('MTTF with optimization: %.2f\n', -fitness);
fprintf('Energy: %.2f J\n', sum(sum(totalPowerProfile * Constants.samplingInterval)));

% Compare
fprintf('MTTF improved by %.2f %%\n', (min(mttf2)/min(mttf1) - 1) * 100);
