% Test: Compare different priority assignments

clear all;
clc;
rng(0);

tol = 0.01; % K
maxit = 10;

solutions = {};

solutions{end + 1} = [];
solutions{end + 1} = [ 0, 1, 7, 4, 10, 10, 11, 4, 8, 6, 26, 27, 9, 25, 16, 4, 22, 20, 11, 14, 37, 19, 43, 34, 32, 15, 20, 37, 44, 43, 25, 21, 40, 17, 27, 38, 47, 22, 30, 28, 48, 50, 4, 28, 50, 26, 37, 39, 42, 34, 45, 49, 57, 55, 31, 50, 52, 57, 48 ];

solutionCount = length(solutions);

[ graph, hotspot ] = setup('004_060');

[ vdd, ngate ] = Utils.collectLeakageParams(graph);

mttf = [];

drawing = figure;

fprintf('%10s%15s%15s\n', 'Solution', 'Lifetime, TU', '+ %');
for i = 1:solutionCount
  priority = solutions{i};

  schedule = LS.process(graph.pes, graph, graph.mapping, priority);
  graph.assignDistributedSchedule(schedule);

  powerProfile = Power.calculateDynamicProfile(graph);

  T = hotspot.solveCondensedEquationWithLeakage(...
    powerProfile, vdd, ngate, tol, maxit);

  Utils.drawSimulation(graph, powerProfile, T);

  mttf(end + 1) = Lifetime.predictCombined(T);

  figure(drawing);

  subplot(solutionCount, 2, 2 * i - 1);
  x = ((1:size(powerProfile, 1)) - 1) * Constants.samplingInterval;
  Utils.drawLines('Temperature', 'Time, s', 'Temperature, C', ...
    x, T - Constants.degreeKelvin);

  subplot(solutionCount, 2, 2 * i);
  Lifetime.drawCycles(T);
  title([ 'Thermal cycles (lifetime ', ...
    num2str(Utils.round2(mttf(end), 0.01)), ' TU)' ]);

  if i == 1
    fprintf('%10d%15.2f%15.2f\n', i, mttf(end), 0);
  else
    fprintf('%10d%15.2f%15.2f\n', i, mttf(end), (mttf(end)/mttf(1) - 1) * 100);
  end
end
