setup;

config = Optima('001_030');

repeat = 1;
timeScale = 1;
powerScale = 1;

param_line = @(leakage, solution) ...
  Utils.configStream(...
    'verbose', 0, ...
    'deadline_ratio', 1, ...
    'leakage', leakage, ...
    'solution', solution, ...
    'time_scale', timeScale, ...
    'power_scale', powerScale);

total = 0;
for i = 1:repeat
  [ T, P, tce ] = Optima.solve(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line(0, 'condensed_equation'));
  total = total + tce;
end
tce = total / repeat;

total = 0;
for i = 1:repeat
  [ Tss, dummy, tss ] = Optima.solve(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line(0, 'precise_steady_state'));
  total = total + tss;
end
tss = total / repeat;

total = 0;
for i = 1:repeat
  [ intervals, Tcce, Pcce, tcce ] = Optima.solve_coarse( ...
    config.system, config.floorplan, config.hotspot, ...
    config.params, param_line(0, 'coarse_condensed_equation'));
  total = total + tce;
end
tce = total / repeat;

fprintf('CE:  %.6f s\n', tce);
fprintf('SS:  %.6f s\n', tss);
fprintf('CCE: %.6f s\n', tcce);

fprintf('CE  / SS: %.2f times\n', tce / tss);
fprintf('CCE / SS: %.2f times\n', tcce / tss);
fprintf('CCE / CE: %.2f times\n', tcce / tce);

[ stepCount, processorCount ] = size(T);

time = ((1:stepCount) - 1) * Constants.samplingInterval;

ctime = cumsum(intervals) - intervals;

fprintf('CCE steps %d instead of %d\n', length(ctime), length(time));

nodeCount = 4 * processorCount + 12;

fprintf('Block equations: %d\n', nodeCount);
fprintf('Total equations: %d\n', nodeCount * stepCount);
fprintf('Block size: %.2f Mb\n', nodeCount^2 * 8 / 1024 / 1024);
fprintf('Total size: %.2f Mb\n', (nodeCount * stepCount)^2 * 8 / 1024 / 1024);
fprintf('Diagonal size: %.2f Mb\n', stepCount * nodeCount^2 * 8 / 1024 / 1024);

figure;

subplot(2, 1, 1);
Utils.drawLines('Power Profile', 'Time, s', 'Power, W', time, P);
set(gca, 'XLim', [ 0 time(end) ]);

Utils.drawLines([], [], [], ctime, Pcce, [], 'Line', '--');

subplot(2, 1, 2);
Utils.drawLines('Temperature Profile', 'Time, s', 'Temperature, C', ...
  time, T - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);

Utils.drawLines([], [], [], ...
  ctime, Tcce - Constants.degreeKelvin, [], 'Line', '--');
