% Test: Testdrive for the GLSA in case of the aging and energy optimization

clear all;
clc;
rng(0);

runTimes = 4;

[ graph, hotspot, dynamicPowerProfile ] = setup('test_cases/test_case_4_60');

vdd = zeros(0);
ngate = zeros(0);

for pe = graph.pes, pe = pe{1};
  vdd(end + 1) = pe.voltage;
  ngate(end + 1) = pe.ngate;
end

tuning = Genetic.LSAgingEnergy.defaultTuning(...
  'generationLimit', 100, ...
  'mobilityCreationFactor', 0.5, ...
  'populationSize', 50, ...
  'crossoverFraction', 0.8, ...
  'mutationProbability', 0.05 ...
);

% First, without any efforts
Utils.startTimer('Solve with the CE');
[ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, vdd, ngate, tuning.leakageTolerance, ...
  tuning.maxLeakageIterations);
Utils.stopTimer();

Utils.drawSimulation(graph, totalPowerProfile, T);

aging0 = Lifetime.predict(T);
energy0 = sum(sum(totalPowerProfile * Constants.samplingInterval));

fprintf('MTTF without optimization: %.2f\n', aging0);
fprintf('Energy without optimization: %.2f J\n', energy0);
fprintf('\n');

rows = floor(sqrt(runTimes));
cols = ceil(runTimes / rows);

figure;

for i = 1:runTimes
  fprintf('Evaluation #%d\n', i);

  drawing = subplot(rows, cols, i);

  line(aging0, energy0, 'Marker', '*', 'MarkerSize', 15, ...
    'Color', 'g', 'LineWidth', 1.1);

  % Now, try to optimize with the GLSA
  ls = Genetic.LSAgingEnergy(graph, hotspot, tuning);

  Utils.startTimer();
  [ priority, fitness, output ] = ls.solve(drawing);
  t = Utils.stopTimer();

  aging = -fitness(:, 1);
  energy = fitness(:, 2);

  [ dummy, I ] = sort(aging);
  line(aging(I), energy(I), 'Color', 'b');

  fprintf('Generations: %d\n', output.generations);
  fprintf('Time: %.2 s\n', t);
  fprintf('\n');
  fprintf('%15s%15s%15s%15s\n', 'Aging, TU', '+ %', 'Energy, J', '+ %');

  for i = 1:length(aging)
    fprintf('%15.2f%15.2f%15.2f%15.2f\n', ...
      aging(i), (aging(i)/aging0 - 1) * 100, energy(i), (energy(i)/energy0 - 1) * 100);
  end

  fprintf('\n\n');
end
