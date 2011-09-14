% Test: Find local minima and maxima on the temperature curves

clear all;
clc;
rng(0);

[ graph, hotspot, dynamicPowerProfile ] = setup('test_cases/004_060');
[ steps, cores ] = size(dynamicPowerProfile);

x = ((1:steps) - 1) * Constants.samplingInterval;

if true
  tol = 0.01; % K
  maxit = 10;

  [ vdd, ngate ] = Utils.collectLeakageParams(graph);

  [ T, i, totaldynamicPowerProfile ] = hotspot.solveCondensedEquationWithLeakage(...
    dynamicPowerProfile, vdd, ngate, tol, maxit);
else
  T = hotspot.solveCondensedEquation(dynamicPowerProfile);
end

tic
mttf = Lifetime.predictMultiple(T);
toc

tic
mttf0 = Lifetime.predictCombined(T);
toc

fprintf('%5s%15s\n', 'Core', 'MTTF, TU');
for i = 1:length(mttf)
  fprintf('%5d%15.2f\n', i, mttf(i));
end

fprintf('%5s%15.2f\n', 'min', min(mttf));
fprintf('%5s%15.2f\n', 'max', max(mttf));
fprintf('%5s%15.2f\n', 'mean', mean(mttf));

fprintf('\n');
fprintf('Alternative way to estimate the overall lifetime: %.2f\n', mttf0);
