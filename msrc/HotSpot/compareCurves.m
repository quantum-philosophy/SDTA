setup;

processorArea = 81e-6;
powerScale = 20;
timeScale = 1 / 734 * 1000;

config = Optima('001_030');
config.changeArea(processorArea);

param_line = Utils.configStream(...
  'deadline_ratio', 1, ...
  'power_scale', powerScale, ...
  'time_scale', timeScale, ...
  'solution', 'condensed_equation', ...
  'verbose', 0, ...
  'leakage', 0);

chunks = 15;

Tce = zeros(0, 0);
Ths = zeros(0, 0);

for i = 1:chunks
  [ chunkTce, dummy, dummy, dummy, chunkThs, dummy ] = ...
    Optima.verify(config.system, config.floorplan, ...
      config.hotspot, config.params, param_line, i, 0);

  Tce = [ Tce; chunkTce ];
  Ths = [ Ths; chunkThs ];
end

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
