setup;

config = Optima('001_030');

processorArea = 81e-6;
powerScale = 10;

config.changeArea(processorArea);

param_line = @(leakage, solution, power_scale) ...
  Utils.configStream(...
    'verbose', 0, ...
    'deadline_ratio', 1, ...
    'leakage', leakage, ...
    'solution', solution, ...
    'power_scale', power_scale);

Tce = Optima.solve(config.system, config.floorplan, config.hotspot, config.params, ...
  param_line(0, 'condensed_equation', powerScale));

Tss = Optima.solve(config.system, config.floorplan, config.hotspot, config.params, ...
  param_line(0, 'steady_state', powerScale));

[ stepCount, processorCount ] = size(Tce);
time = ((1:stepCount) - 1) * config.samplingInterval;

Tce = Tce - Constants.degreeKelvin;
Tss = Tss - Constants.degreeKelvin;

figure;

subplot(2, 1, 1);
Utils.compareLines('Steady-State Dynamic Temperature Curve', ...
  'Time, s', 'Temperature, C', time, 'CE', Tce, 'SS', Tss);

tgff = TestCase.TGFF(config.tgff);
graph = tgff.graphs{1};
pes = tgff.pes;
LS.mapEarliestAndSchedule(graph, pes);
graph.assignDeadline(graph.duration);

subplot(2, 1, 2);
graph.draw(false);
