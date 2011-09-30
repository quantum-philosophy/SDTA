% Test: Compare different priority assignments

clear all;
clc;
rng(0);

tol = 0.01; % K
maxit = 10;

solutions = {};

solutions{end + 1} = [];
solutions{end + 1} = [ 0, 1, 8, 3, 2, 6, 4, 12, 13, 10, 17, 33, 12, 21, 22, 22, 20, 21, 19, 38, 28, 19, 53, 29, 37, 42, 18, 35, 51, 49, 18, 57, 37, 36, 34, 33, 18, 54, 43, 45, 20, 44, 6, 33, 50, 23, 18, 15, 56, 41, 45, 49, 54, 36, 53, 29, 54, 38, 54 ];

solutionCount = length(solutions);

[ graph, hotspot ] = setup('004_060');

[ vdd, ngate ] = Utils.collectLeakageParams(graph);

mttf = [];

drawing = figure;

fprintf('%10s%15s%15s\n', 'Solution', 'Lifetime, TU', '+ %');
for i = 1:solutionCount
  priority = solutions{i};

  LS.schedule(graph, priority);

  powerProfile = Power.calculateDynamicProfile(graph);

  T = hotspot.solveCondensedEquationWithLeakage(...
    powerProfile, vdd, ngate, tol, maxit);

  Utils.drawSimulation(graph, powerProfile, T);

  mttf(end + 1) = Lifetime.predictCombined(T);

  figure(drawing);

  time = size(powerProfile, 1) * Constants.samplingInterval;

  subplot(solutionCount, 2, 2 * i - 1);
  x = ((1:size(powerProfile, 1)) - 1) * Constants.samplingInterval;
  Utils.drawLines('Temperature', 'Time, s', 'Temperature, C', ...
    x, T - Constants.degreeKelvin);

  set(gca, 'XLim', [ 0, time ]);

  subplot(solutionCount, 2, 2 * i);
  Lifetime.drawCycles(T);
  title([ 'Thermal cycles (lifetime ', ...
    num2str(Utils.round2(mttf(end), 0.01)), ' TU)' ]);

  set(gca, 'XLim', [ 0, time ]);

  if i == 1
    fprintf('%10d%15.2f%15.2f\n', i, mttf(end), 0);
  else
    fprintf('%10d%15.2f%15.2f\n', i, mttf(end), (mttf(end)/mttf(1) - 1) * 100);
  end
end
