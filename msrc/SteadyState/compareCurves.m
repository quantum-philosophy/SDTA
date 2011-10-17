setup;

config = Optima('001_030');

processorArea = 81e-6;
powerScale = 10;
time_scale = 1;

config.changeArea(processorArea);

ratio = (37.5 * 37.5) / 81;

% Die
dieArea = processorArea * config.processorCount;
dieSide = sqrt(dieArea);
% Sink
sinkSide = sqrt(ratio * dieArea);
% Spreader
spreaderSide = (dieSide + sinkSide) / 2;

hotspot_line = sprintf( ...
  's_sink %.4f s_spreader %.4f', ...
  sinkSide, spreaderSide);

param_line = @(leakage, solution, power_scale) ...
  Utils.configStream(...
    'hotspot', hotspot_line, ...
    'verbose', 0, ...
    'deadline_ratio', 1, ...
    'leakage', leakage, ...
    'solution', solution, ...
    'time_scale', time_scale, ...
    'power_scale', power_scale);

[ Tce, power ] = Optima.solve(config.system, config.floorplan, config.hotspot, config.params, ...
  param_line(0, 'condensed_equation', powerScale));

Tss = Optima.solve(config.system, config.floorplan, config.hotspot, config.params, ...
  param_line(0, 'steady_state', powerScale));

[ stepCount, processorCount ] = size(Tce);
time = ((1:stepCount) - 1) * config.samplingInterval;

Pmax = max(max(power));
fprintf('Maximal power consumption: %.2f W\n', Pmax);

Tce = Tce - Constants.degreeKelvin;
Tss = Tss - Constants.degreeKelvin;

figure;

subplot(2, 1, 1);
Utils.compareLines('Steady-State Dynamic Temperature Curve', ...
  'Time, s', 'Temperature, C', time, ...
  'SSDTC', Tce, 'Steady-State Approximation', Tss);

tgff = TestCase.TGFF(config.tgff);
graph = tgff.graphs{1};
pes = tgff.pes;
LS.mapEarliestAndSchedule(graph, pes);
graph.assignDeadline(graph.duration);
graph.scale(time_scale);

subplot(2, 1, 2);
graph.draw(false);

graph.inspect();
