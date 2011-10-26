setup;

repeat = 10;

config = Optima('001_030');

param_line = Utils.configStream(...
  'time_scale', 1, ...
  'verbose', 0, ...
  'solution', 'condensed_equation', ...
  'leakage', 0);

power = Optima.get_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line);

[ stepCount, processorCount ] = size(power);

fprintf('Steps: %d\n', stepCount);
fprintf('Processor: %d\n', processorCount);

time = 0;
for i = 1:repeat
  [ T, t ] = Optima.solve_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line, power);
  time = time + t;
end
time = time / repeat;

fprintf('CE: %.4f s\n', time);

Utils.drawTemperature(T, 'Temperature', config.samplingInterval, ...
  [], 'Color', 'r');

time = 0;
for i = 1:repeat
  [ d, d, d, T, t ] = Optima.verify_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line, power, 1, 0);
  time = time + t;
end
time = time / repeat;

fprintf('HS: %.4f s (as was)\n', time);

Utils.drawTemperature(T, [], config.samplingInterval, ...
  [], 'Color', 'g');

param_line = Utils.configStream(...
    'verbose', 0, ...
    'solution', 'hotspot', ...
    'max_iterations', 1, ...
    'tolerance', 0.1, ...
    'leakage', 0);

time = 0;
for i = 1:repeat
  [ T, t ] = Optima.solve_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line, power);
  time = time + t;
end
time = time / repeat;

fprintf('HS: %.4f s (once again)\n', time);

param_line = Utils.configStream(...
    'verbose', 0, ...
    'solution', 'hotspot', ...
    'max_iterations', 1, ...
    'tolerance', 0, ...
    'leakage', 0);

time = 0;
for i = 1:repeat
  [ T, t ] = Optima.solve_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line, power);
  time = time + t;
end
time = time / repeat;

fprintf('HS: %.4f s (without control)\n', time);

extendedPower = [ power, zeros(stepCount, 3 * processorCount + 12) ];

time = 0;
for i = 1:repeat
  [ T, t ] = Optima.solve_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line, extendedPower);
  time = time + t;
end
time = time / repeat;

fprintf('HS: %.4f s (without control and without extension)\n', time);

param_line = Utils.configStream(...
    'verbose', 0, ...
    'solution', 'transient_analytical', ...
    'leakage', 0);

time = 0;
for i = 1:repeat
  [ T, t ] = Optima.solve_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line, power);
  time = time + t;
end
time = time / repeat;

fprintf('AT: %.4f s\n', time);

Utils.drawTemperature(T, [], config.samplingInterval, ...
  [], 'Color', 'b');
