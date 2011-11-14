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

fprintf('Energy CE: %.4f\n', sum(sum(Pce)) * config.samplingInterval);
fprintf('Energy SS: %.4f\n', sum(sum(Pss)) * config.samplingInterval);

MTTFce1 = min(Lifetime.predictMultipleAndDraw(T, config.samplingInterval));
MTTFss1 = min(Lifetime.predictMultipleAndDraw(Tss, config.samplingInterval));

MTTFce2 = Optima.predict(T, config.samplingInterval);
MTTFss2 = Optima.predict(Tss, config.samplingInterval);

fprintf('MTTF CE in Matlab: %.4f\n', MTTFce1);
fprintf('MTTF SS in Matlab: %.4f\n', MTTFss1);

fprintf('MTTF CE in C++: %.4f\n', MTTFce2);
fprintf('MTTF SS in C++: %.4f\n', MTTFss2);
