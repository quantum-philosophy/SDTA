clear all;
clc;

name = '004_060';

tgff = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
hotspot = Utils.path('hotspot.config');
floorplan = Utils.path([ name, '_temp.flp' ]);
params = Utils.path('parameters.config');

processorCount = Utils.readParameter(tgff, 'table_cnt');

processorArea = [ 1e-6, 2e-6, 4e-6, 8e-6, 16e-6, 32e-6, 64e-6, 128e-6 ];

Error = zeros(0, processorCount);
Scale = zeros(0, 0);

scale = 1;

config = @(steady_state, power_scale) ...
  Utils.configStream(...
    'verbose', 0, ...
    'deadline_ratio', 1, ...
    'leakage', 0, ...
    'steady_state', steady_state, ...
    'power_scale', power_scale);

fprintf('%15s%15s%15s%15s%15s%15s%15s%15s\n', 'Area, mm^2', 'Conductance', ...
  'Capacitance', 'Power scale', 'T min, C', 'T mean, C', 'T max, C', 'RMSE');

for i = 1:length(processorArea)
  area = processorArea(i);

  Utils.generateFloorplan(floorplan, processorCount, area);

  [ G, C ] = Optima.get_coefficients(system, floorplan, hotspot, params);

  if i == 1
    [ temperature1, power ] = Optima.solve(...
      system, floorplan, hotspot, params, config(0, scale));
    meanT = mean(mean(temperature1));
    standard = meanT;
  else
    while true
      [ temperature1, power ] = Optima.solve(...
        system, floorplan, hotspot, params, config(0, scale));

      meanT = mean(mean(temperature1));
      if meanT >= standard, break; end;

      scale = scale + 0.01;
    end
  end

  meanT = meanT - Constants.degreeKelvin;
  minT = min(min(temperature1)) - Constants.degreeKelvin;
  maxT = max(max(temperature1)) - Constants.degreeKelvin;

  temperature2 = Optima.solve(system, floorplan, hotspot, params, config(1, scale));

  stepCount = size(temperature1, 1);
  error = sqrt(sum((temperature2 - temperature1).^ 2) / stepCount);

  meanError = mean(error);

  fprintf('%15d%15.4f%15.2e%15.2f%15.2f%15.2f%15.2f%15.2f\n', ...
    area * 1e6, G(1, 1), C(1), scale, minT, meanT, maxT, meanError);

  Error(end + 1, :) = error;
  Scale(end + 1) = scale;
end

processorArea = processorArea * 1e6;

figure;

Utils.drawLines('Temperature RMSE', ...
  'Area of one core, mm^2', 'RMSE', ...
  processorArea, Error, [], 'Line', '--', 'Color', 'k');

x = processorArea(1):1:processorArea(end);
y = interp1(processorArea, mean(Error, 2), x, 'cubic');
line(x, y, 'Color', Constants.roundRobinColors{1}, 'LineWidth', 2);

figure;

Utils.drawLines('Power Scale', ...
  'Area of one core, mm^2', 'Scale', ...
  processorArea, transpose(Scale), [], 'Marker', 'o');
