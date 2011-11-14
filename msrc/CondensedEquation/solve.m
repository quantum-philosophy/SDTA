setup;

config = Optima('001');

repeat = 1;

param_line = @(solution) ...
  Utils.configStream(...
    'verbose', 0, ...
    'solution', solution);

total = 0;
for i = 1:repeat
  [ T, Pce, tce ] = Optima.solve(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line('condensed_equation'));
  total = total + tce;
end
tce = total / repeat;

total = 0;
for i = 1:repeat
  [ Tss, Pss, tss ] = Optima.solve(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line('precise_steady_state'));
  total = total + tss;
end
tss = total / repeat;

fprintf('CE: %.6f s\n', tce);
fprintf('SS: %.6f s\n', tss);
fprintf('CE / SS: %.2f times\n', tce / tss);

[ stepCount, processorCount ] = size(T);

time = ((1:stepCount) - 1) * Constants.samplingInterval;

figure;

subplot(3, 1, 1);
Utils.drawLines('Power Profile', 'Time, s', 'Power, W', time, Pce);
set(gca, 'XLim', [ 0 time(end) ]);

subplot(3, 1, 2);
Utils.drawLines('SSDTC with CE', 'Time, s', 'Temperature, C', ...
  time, T - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);

subplot(3, 1, 3);
Utils.drawLines('SSDTC with SS', 'Time, s', 'Temperature, C', ...
  time, Tss - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);

MTTFce = min(Lifetime.predictMultipleAndDraw(T, config.samplingInterval));
MTTFss = min(Lifetime.predictMultipleAndDraw(Tss, config.samplingInterval));

fprintf('MTTF CE: %.4f\n', MTTFce);
fprintf('MTTF SS: %.4f\n', MTTFss);

fprintf('Energy CE: %.4f\n', sum(sum(Pce)) * config.samplingInterval);
fprintf('Energy SS: %.4f\n', sum(sum(Pss)) * config.samplingInterval);
