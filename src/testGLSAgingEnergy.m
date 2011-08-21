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

[ mttf, cycles ] = Lifetime.predict(T);

aging = min(mttf);
energy = sum(sum(totalPowerProfile * Constants.samplingInterval));

fprintf('MTTF without optimization: %.2f\n', aging);
fprintf('Energy: %.2f J\n', energy);

drawing = figure;

line(aging, energy, 'Marker', '*', 'MarkerSize', 15, ...
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

fprintf('Number of generation: %d\n', output.generations);
