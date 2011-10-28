setup;

config = Optima('001_030');

stepCount = 100;
processorCount = 64;
processorArea = 4e-6;

config.changeProcessorCountAndArea(processorCount, processorArea);
config.scalePackage();

power = ones(stepCount, processorCount);

repeat = 10;
timeScale = 1;
powerScale = 1;

param_line = @(leakage, solution) ...
  Utils.configStream(...
    'verbose', 0, ...
    'deadline_ratio', 1, ...
    'leakage', leakage, ...
    'solution', solution, ...
    'time_scale', timeScale, ...
    'power_scale', powerScale, ...
    'max_iterations', 1, ...
    'tolerance', 0);

total1 = 0;
total2 = 0;
for i = 1:repeat
  tic;
  [ T, tce ] = Optima.solve_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line(0, 'condensed_equation'), power);
  total1 = total1 + toc;
  total2 = total2 + tce;
end
tce1 = total1 / repeat;
tce2 = total2 / repeat;

fprintf('Condensed Equation:\n');
fprintf('In Matlab:  %.6f s\n', tce1);
fprintf('In C:       %.6f s\n', tce2);
fprintf('Difference: %.6f s\n', tce1 - tce2);

total1 = 0;
total2 = 0;
for i = 1:repeat
  tic;
  [ T, tce ] = Optima.solve_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line(0, 'hotspot'), power);
  total1 = total1 + toc;
  total2 = total2 + tce;
end
tce1 = total1 / repeat;
tce2 = total2 / repeat;

fprintf('\nHotspot:\n');
fprintf('In Matlab:  %.6f s\n', tce1);
fprintf('In C:       %.6f s\n', tce2);
fprintf('Difference: %.6f s\n', tce1 - tce2);
