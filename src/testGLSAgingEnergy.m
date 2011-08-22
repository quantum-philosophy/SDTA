% Test: Testdrive for the GLSA in case of the aging and energy optimization

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

% First, without any efforts
Utils.startTimer('Solve with the CE');
[ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage( ...
  dynamicPowerProfile, vdd, ngate, Genetic.LS.leakageTolerance, ...
  Genetic.LS.maxLeakageIterations);
Utils.stopTimer();

Utils.drawSimulation(graph, totalPowerProfile, T);

[ mttf, cycles ] = Lifetime.predict(T);

aging0 = min(mttf);
energy0 = sum(sum(totalPowerProfile * Constants.samplingInterval));

fprintf('MTTF without optimization: %.2f\n', aging0);
fprintf('Energy: %.2f J\n', energy0);

drawing = figure;

line(aging0, energy0, 'Marker', '*', 'MarkerSize', 15, ...
  'Color', 'g', 'LineWidth', 1.1);

% Now, try to optimize with the GLSA
ls = Genetic.LSAgingEnergy(graph, hotspot);

Utils.startTimer('Solve with the GLSA');
[ priority, fitness, output ] = ls.solve(drawing);
Utils.stopTimer();

aging = -fitness(:, 1);
energy = fitness(:, 2);

[ dummy, I ] = sort(aging);
line(aging(I), energy(I), 'Color', 'b');

fprintf('%15s%15s%15s%15s\n', 'MFFT', 'd(MTTF), %', 'Energy', 'd(Energy), %');

for i = 1:length(aging)
  fprintf('%15.2f%15.2f%15.2f%15.2f\n', ...
    aging(i), (aging(i)/aging0 - 1) * 100, energy(i), (energy(i)/energy0 - 1) * 100);
end

fprintf('Number of generation: %d\n', output.generations);
