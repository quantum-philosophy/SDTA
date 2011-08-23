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
  'generationStall', 50, ...
  'generationTolerance', 0.01, ...
  'generationLimit', 500, ...
  'mobilityCreationFactor', 0.5, ...
  'populationSize', 100, ...
  'generationalGap', 0.4, ...
  'crossoverFraction', 0.6, ...
  'mutationProbability', 'max(0.2, 1 / exp(state.Generation * 0.05))' ...
);

% First, without any efforts
Utils.startTimer('Solve with the CE');
[ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, vdd, ngate, tuning.leakageTolerance, ...
  tuning.maxLeakageIterations);
Utils.stopTimer();

[ mttf, cycles ] = Lifetime.predict(T);

aging0 = min(mttf);
energy0 = sum(sum(totalPowerProfile * Constants.samplingInterval));

fprintf('MTTF without optimization: %.2f\n', aging0);
fprintf('Energy: %.2f J\n', energy0);

drawing = figure;

line(0, aging0, 'Marker', '*', 'MarkerSize', 15, ...
  'Color', 'g', 'LineWidth', 1.1);

% Now, try to optimize with the GLSA
ls = Genetic.LSAging(graph, hotspot, tuning);

Utils.startTimer('Solve with the GLSA');
[ priority, fitness, output ] = ls.solve(drawing);
Utils.stopTimer();

% Calculate the best one
LS.schedule(graph, priority);
dynamicPowerProfile = Power.calculateDynamicProfile(graph);
[ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, vdd, ngate, tuning.leakageTolerance, ...
  tuning.maxLeakageIterations);

[ mttf, cycles ] = Lifetime.predict(T);

aging = min(mttf);
energy = sum(sum(totalPowerProfile * Constants.samplingInterval));

fprintf('MTTF with optimization: %.2f\n', aging);
fprintf('Energy: %.2f J\n', energy);

% Compare
fprintf('MTTF improvement: %.2f %%\n', (aging / aging0 - 1) * 100);
