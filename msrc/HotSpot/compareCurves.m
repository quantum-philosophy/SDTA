setup;

chunks = 30;

spreaderRatio = 37.5 / 9;

processorArea = 49e-6;
maxPower = 35;

totalTime = 1;

config = Optima('001_030');

config.changeArea(processorArea);
[ sinkSide, spreaderSide, dieSide, sinkThickness ] = ...
  config.scalePackage(spreaderRatio);

fprintf('Die side: %.2f mm\n', dieSide * 1e3);
fprintf('Spreader side: %.2f mm\n', spreaderSide * 1e3);
fprintf('Sink side: %.2f mm\n', sinkSide * 1e3);
fprintf('Sink thickness: %.2f mm\n', sinkThickness * 1e3);

power = Optima.get_power(config.system, config.floorplan, ...
  config.hotspot, config.params, 'deadline_ratio 1');

[ stepCount, processorCount ] = size(power);

param_line = @(solution, max_iterations) ...
  Utils.configStream(...
    'deadline_ratio', 1, ...
    'max_iterations', max_iterations, ...
    'tolerance', 0, ...
    'power_scale', maxPower / max(max(power)), ...
    'time_scale', totalTime / (config.samplingInterval * stepCount), ...
    'solution', solution, ...
    'hotspot', 'r_convec 0.1', ...
    'verbose', 0, ...
    'leakage', 'exponential');

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
    config.params, param_line('hotspot', i)) - Constants.degreeKelvin;

  Error = [ Error; Utils.NRMSE(chunkTce, chunkThs) * 100 ];

  Tce = [ Tce; chunkTce ];
  Ths = [ Ths; chunkThs ];
end

fprintf('RMSE: %.2f\n', Utils.RMSE(chunkTce, chunkThs));
fprintf('NRMSE: %.2f%%\n', Utils.NRMSE(chunkTce, chunkThs) * 100);

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
