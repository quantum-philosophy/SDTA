setup;

chunks = 20;

totalTime = 1;
maxPower = 20;
processorArea = 4e-6;

config = Optima('001');

config.changeArea(processorArea);
[ sinkSide, spreaderSide, dieSide, sinkThickness ] = config.standardPackage();

fprintf('%20s%20s%20s%20s%20s\n', ...
  'Area, mm^2', 'Die, mm', 'Spreader, mm', 'Sink, mm', 'Thickness, mm');

fprintf('%20.2f%20.2f%20.2f%20.2f%20.2f\n', ...
  processorArea * 1e6, dieSide * 1e3, spreaderSide * 1e3, sinkSide * 1e3, ...
  sinkThickness * 1e3);

power = Optima.get_power(config.system, config.floorplan, ...
  config.hotspot, config.params, 'deadline_ratio 1');

[ stepCount, processorCount ] = size(power);

timeScale = totalTime / (config.samplingInterval * stepCount);
powerScale = maxPower / max(max(power));

param_line = @(solution, max_iterations) ...
  Utils.configStream(...
    'deadline_ratio', 1, ...
    'max_iterations', max_iterations, ...
    'tolerance', 0, ...
    'power_scale', powerScale, ...
    'time_scale', timeScale, ...
    'solution', solution, ...
    'verbose', 0, ...
    'leakage', '');

Tce = zeros(0, 0);
Ths = zeros(0, 0);

Error = zeros(0);

[ chunkTce, power ] = Optima.solve(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line('condensed_equation', 0));

chunkTce = chunkTce - Constants.degreeKelvin;

fprintf('Application time: %.2f s\n', size(power, 1) * config.samplingInterval);
fprintf('P max: %.2f W\n', max(max(power)));
fprintf('T max: %.2f C\n', max(max(chunkTce)));
fprintf('T min: %.2f C\n', min(min(chunkTce)));

for i = 1:chunks
  chunkThs = Optima.solve(config.system, config.floorplan, config.hotspot, ...
    config.params, param_line('transient_analytical', i)) - Constants.degreeKelvin;

  Error = [ Error; Utils.NRMSE(chunkTce, chunkThs) * 100 ];

  Tce = [ Tce; chunkTce ];
  Ths = [ Ths; chunkThs ];
end

fprintf('First NRMSE: %.2f%%\n', Error(1));
fprintf('Last NRMSE: %.2f%%\n', Error(end));

[ stepCount, processorCount ] = size(Tce);
time = ((1:stepCount) - 1) * config.samplingInterval;

figure;

options = struct( ...
  'xlabel', 'Time, s', ...
  'ylabel', 'Temperature, C');
options.legend = { 'Repeating SSDTC', 'Iterative HotSpot Simulation' };

Utils.draw(time, [ Tce, Ths ], options);

chunkLength = length(Tce) / chunks;

lims = get(gca, 'YLim');

for i = 1:(chunks - 1)
  t = time(chunkLength * i);
  line([ t, t ], lims, 'Line', '--', 'Color', [ 64, 64, 64 ] / 255);
end

figure;

options = struct( ...
  'xlabel', 'Iterations', ...
  'ylabel', 'Normalized RMSE, %', ...
  'marker', true);
options.legend = { 'NRMSE' };

Utils.draw(1:chunks, Error, options);
