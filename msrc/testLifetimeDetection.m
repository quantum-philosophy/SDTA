% Test: Find local minima and maxima on the temperature curves

clear all;
clc;
rng(0);

[ graph, hotspot, powerProfile ] = setup('test_cases/004_010');
[ steps, cores ] = size(powerProfile);

x = ((1:steps) - 1) * Constants.samplingInterval;

T = hotspot.solveCondensedEquation(powerProfile);

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
