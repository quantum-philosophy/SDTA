setup;

config = Optima('001');

param_line = @(solution) ...
  Utils.configStream(...
    'deadline_ratio', 1, ...
    'verbose', 0, ...
    'leakage', '', ...
    'solution', solution);

[ T, Pce, tce ] = Optima.solve(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line('condensed_equation'));

hotspot = Hotspot(config.floorplan, config.hotspot, '');
T2 = hotspot.solve(Pce, 'bc');

[ stepCount, processorCount ] = size(T);

time = ((1:stepCount) - 1) * Constants.samplingInterval;

figure;

subplot(3, 1, 1);
Utils.drawLines('Power Profile', 'Time, s', 'Power, W', time, Pce);
set(gca, 'XLim', [ 0 time(end) ]);

subplot(3, 1, 2);
Utils.drawLines('SSDTC', 'Time, s', 'Temperature, C', ...
  time, T - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);

subplot(3, 1, 3);
Utils.drawLines('Alternative SSDTC', 'Time, s', 'Temperature, C', ...
  time, T2 - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);
