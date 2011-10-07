clear all;
clc;

name = '001_030';
processorArea = 128e-6;
powerScale = 29.70;

tgff = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
hotspot = Utils.path('hotspot.config');
floorplan = Utils.path([ name, '_temp.flp' ]);
params = Utils.path('parameters.config');

processorCount = Utils.readParameter(tgff, 'table_cnt');
Utils.generateFloorplan(floorplan, processorCount, processorArea);

temperature1 = Optima.solve(system, floorplan, hotspot, params, ...
  sprintf('deadline_ratio 1 \n leakage 0 \n steady_state 0 \n power_scale %f', powerScale));

temperature2 = Optima.solve(system, floorplan, hotspot, params, ...
  sprintf('deadline_ratio 1 \n leakage 0 \n steady_state 1 \n power_scale %f', powerScale));

samplingInterval = Utils.readParameter(hotspot, '-sampling_intvl');

[ stepCount, processorCount ] = size(temperature1);
time = ((1:stepCount) - 1) * samplingInterval;

figure;

subplot(3, 1, 1);
Utils.drawLines('SSDTC via the SS approximation', 'Time, s', 'Temperature, C', ...
  time, temperature2 - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);
YLim = get(gca, 'YLim');
set(gca, 'YLim', YLim);

subplot(3, 1, 2);
Utils.drawLines('SSDTC via the CE method', 'Time, s', 'Temperature, C', ...
  time, temperature1 - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);
set(gca, 'YLim', YLim);

difference = temperature2 - temperature1;

subplot(3, 1, 3);
Utils.drawLines('Difference', 'Time, s', 'Temperature, C', ...
  time, difference);
line([ time(1) time(end) ], [ 0, 0 ], 'Line', '--', 'Color', 'k');
set(gca, 'XLim', [ 0 time(end) ]);

variance = sum(difference .^ 2) / (stepCount - 1);

fprintf('%10s%15s\n', 'Processor', 'Variance');
for i = 1:processorCount
  fprintf('%10d%15.2f\n', i, variance(i));
end
