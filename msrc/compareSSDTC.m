clear all;
clc;

name = '001_030';
processorArea = 4e-6;
powerScale = 1;

tgffopt = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
hotspot = Utils.path('hotspot.config');
floorplan = Utils.path([ name, '_temp.flp' ]);
params = Utils.path('parameters.config');

processorCount = Utils.readParameter(tgffopt, 'table_cnt');
Utils.generateFloorplan(floorplan, processorCount, processorArea);

config = @(leakage, steady_state, power_scale) ...
  Utils.configStream(...
    'verbose', 0, ...
    'deadline_ratio', 1, ...
    'leakage', leakage, ...
    'steady_state', steady_state, ...
    'power_scale', power_scale);

Tce = Optima.solve(system, floorplan, hotspot, params, ...
  config(0, 0, powerScale));

Tss = Optima.solve(system, floorplan, hotspot, params, ...
  config(0, 1, powerScale));

samplingInterval = Utils.readParameter(hotspot, '-sampling_intvl');

[ stepCount, processorCount ] = size(Tce);
time = ((1:stepCount) - 1) * samplingInterval;

Tce = Tce - Constants.degreeKelvin;
Tss = Tss - Constants.degreeKelvin;

figure;

subplot(2, 1, 1);
Utils.compareLines('Steady-State Dynamic Temperature Curve', ...
  'Time, s', 'Temperature, C', time, 'CE', Tce, 'SS', Tss);

tgff = Utils.path([ name, '.tgff' ]);
tgff = TestCase.TGFF(tgff);
graph = tgff.graphs{1};
pes = tgff.pes;
LS.mapEarliestAndSchedule(graph, pes);
graph.assignDeadline(graph.duration);

subplot(2, 1, 2);
graph.draw(false);
