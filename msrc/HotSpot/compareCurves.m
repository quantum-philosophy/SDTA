setup;

processorArea = 16e-6;
chunks = 30;
maxPower = 10;
totalTime = 1;

config = Optima('001_030');

config.changeArea(processorArea);
[ sinkSide, spreaderSide, dieSide ] = config.scalePackage();


fprintf('Sink side: %.2f mm\n', sinkSide * 1e3);
fprintf('Spreader side: %.2f mm\n', spreaderSide * 1e3);
fprintf('Die side: %.2f mm\n', dieSide * 1e3);

power = Optima.get_power(config.system, config.floorplan, ...
  config.hotspot, config.params, 'deadline_ratio 1');

[ stepCount, processorCount ] = size(power);

param_line = Utils.configStream(...
  'deadline_ratio', 1, ...
  'power_scale', maxPower / max(max(power)), ...
  'time_scale', totalTime / (stepCount * config.samplingInterval), ...
  'solution', 'condensed_equation', ...
  'hotspot', 'r_convec 0.5', ...
  'verbose', 0, ...
  'leakage', '');

Tce = zeros(0, 0);
Ths = zeros(0, 0);

Error = zeros(0, 2);

for i = 1:chunks
  [ chunkTce, dummy, power, dummy, chunkThs, dummy ] = ...
    Optima.verify(config.system, config.floorplan, ...
      config.hotspot, config.params, param_line, i, 0);

  Error(end + 1, 1:2) = [ Utils.RMSE(chunkTce, chunkThs), max(max(abs(chunkTce - chunkThs))) ];

  Tce = [ Tce; chunkTce ];
  Ths = [ Ths; chunkThs ];
end

fprintf('Application time: %.2f s\n', size(power, 1) * config.samplingInterval);
fprintf('Maximal power: %.2f W\n', max(max(power)));

[ stepCount, processorCount ] = size(Tce);
time = ((1:stepCount) - 1) * config.samplingInterval;

Tce = Tce - Constants.degreeKelvin;
Ths = Ths - Constants.degreeKelvin;

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
  'ylabel', 'Temperature, C', ...
  'marker', true);
options.legend = { 'Root Mean Squared Error', 'Absolute Error' };

Utils.draw(1:chunks, Error, options);
