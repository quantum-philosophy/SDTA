clear all;
clc;
rng(0);

name = '004_060';

system = Utils.path([ name, '.sys' ]);
floorplan = Utils.path([ name, '.flp' ]);
hotspot = Utils.path('hotspot.config');
params = Utils.path('parameters.config');

[ temperature, power ] = Optima.solve(system, floorplan, hotspot, params);
[ stepCount, processorCount ] = size(temperature);

Lifetime.drawCycles(temperature);

tic
mttf = Lifetime.predictMultiple(temperature);
toc

tic
mttf0 = Lifetime.predictCombined(temperature);
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
