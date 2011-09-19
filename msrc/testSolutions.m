% Test: Compare different priority assignments

clear all;
clc;
rng(0);

tol = 0.01; % K
maxit = 10;

solutions = {};

solutions{end + 1} = [];
solutions{end + 1} = [ 0, 1, 8, 2, 5, 5, 4, 12, 27, 7, 22, 9, 12, 33, 14, 4, 18, 18, 14, 29, 31, 31, 33, 46, 28, 37, 47, 32, 37, 52, 26, 44, 43, 45, 45, 47, 26, 38, 40, 52, 49, 27, 34, 44, 51, 26, 19, 45, 35, 37, 52, 30, 54, 45, 48, 13, 55, 56, 41 ];

solutionCount = length(solutions);

[ graph, hotspot ] = setup('004_060');

[ vdd, ngate ] = Utils.collectLeakageParams(graph);

mttf = [];

figure;

fprintf('%10s%15s%15s\n', 'Solution', 'Lifetime, TU', '+ %');
for i = 1:solutionCount
  priority = solutions{i};

  schedule = LS.process(graph.pes, graph, graph.mapping, priority);
  graph.assignDistributedSchedule(schedule);

  Utils.inspectVector('Priority', priority);
  Utils.inspectVector('Schedule', graph.schedule - 1);
  fprintf('Deadline: %f\n', graph.deadline);

  powerProfile = Power.calculateDynamicProfile(graph);

  T = hotspot.solveCondensedEquationWithLeakage(...
    powerProfile, vdd, ngate, tol, maxit);

  mttf(end + 1) = Lifetime.predictCombined(T);

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
