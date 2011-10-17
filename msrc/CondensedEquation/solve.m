setup;

config = Optima('001_030');

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
    'power_scale', powerScale);

total = 0;
for i = 1:repeat
  [ T, P, tce ] = Optima.solve(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line(0, 'condensed_equation'));
  total = total + tce;
end
tce = total / repeat;
fprintf('CE: %.6f s\n', tce);

total = 0;
for i = 1:repeat
  [ Tss, dummy, tss ] = Optima.solve(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line(0, 'steady_state'));
  total = total + tss;
end
tss = total / repeat;
fprintf('SS: %.6f s\n', tss);

fprintf('CE / SS: %.2f times\n', tce / tss);

[ stepCount, processorCount ] = size(T);

time = ((1:stepCount) - 1) * Constants.samplingInterval;

figure;

subplot(2, 1, 1);
Utils.drawLines('Power Profile', 'Time, s', 'Power, W', time, P);
set(gca, 'XLim', [ 0 time(end) ]);

subplot(2, 1, 2);
Utils.drawLines('Temperature Profile', 'Time, s', 'Temperature, C', ...
  time, T - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);
