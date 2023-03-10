% Test: Testdrive for the genetic algorithm for scheduling

clear all;
clc;
rng(0);

silent = true;
runTimes = 9;

[ graph, hotspot, dynamicPowerProfile ] = setup('test_cases/004_060');

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
  'populationSize', 50, ...
  'generationalGap', 0.5, ...
  'crossoverFraction', 0.8, ...
  'mutationProbability', 0.05 ...
);

% First, without any efforts
Utils.startTimer('Solve with the CE');
[ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, vdd, ngate, tuning.leakageTolerance, ...
  tuning.maxLeakageIterations);
Utils.stopTimer();

aging0 = Lifetime.predict(T);
energy0 = sum(sum(totalPowerProfile * Constants.samplingInterval));

fprintf('MTTF without optimization: %.2f\n', aging0);
fprintf('Energy without optimization: %.2f J\n', energy0);

rows = floor(sqrt(runTimes));
cols = ceil(runTimes / rows);

if ~silent, figure; end

fprintf('\n');
fprintf('%5s%15s%15s%15s%15s%15s%15s\n', ...
  'No', 'Generations', 'Time, m', ...
  'Aging, TU', '+ %', ...
  'Energy, J', '+ %');

generation = [];
time = [];
aging = [];
energy = [];

for i = 1:runTimes
  % Now, try to optimize with the GLSA
  ls = Genetic.LSAging(graph, hotspot, tuning);

  if ~silent
    drawing = subplot(rows, cols, i);

    line(0, aging0, 'Marker', '*', 'MarkerSize', 15, ...
      'Color', 'g', 'LineWidth', 1.1);

    Utils.startTimer();
    [ priority, fitness, output ] = ls.solve(drawing);
    time(end + 1) = Utils.stopTimer();
  else
    Utils.startTimer();
    [ priority, fitness, output ] = ls.solve();
    time(end + 1) = Utils.stopTimer();
  end

  % Calculate the best one
  LS.schedule(graph, priority);

  dynamicPowerProfile = Power.calculateDynamicProfile(graph);
  [ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage( ...
    dynamicPowerProfile, vdd, ngate, tuning.leakageTolerance, ...
    tuning.maxLeakageIterations);

  generation(end + 1) = output.generations;
  aging(end + 1) = Lifetime.predict(T);
  energy(end + 1) = sum(sum(totalPowerProfile * Constants.samplingInterval));

  fprintf('%5d%15d%15.2f%15.2f%15.2f%15.2f%15.2f\n', ...
    i, generation(end), time(end) / 60, ...
    aging(end), (aging(end) / aging0 - 1) * 100, ...
    energy(end), (energy(end) / energy0 - 1) * 100);
end

fprintf('%5s%15d%15.2f%15.2f%15.2f%15.2f%15.2f\n', ...
  '~', round(mean(generation)), mean(time) / 60, ...
  mean(aging), mean((aging / aging0 - 1) * 100), ...
  mean(energy), mean((energy / energy0 - 1) * 100));
