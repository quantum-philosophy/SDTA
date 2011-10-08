clear all;
clc;

name = '004_060';

tgff = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
hotspot = Utils.path('hotspot.config');
floorplan = Utils.path([ name, '.flp' ]);
params = Utils.path('parameters.config');

processorCount = Utils.readParameter(tgff, 'table_cnt');

step = 0.01e-3;
processorThickness = 0.10e-3:step:0.50e-3;

Error = zeros(0, processorCount);
Scale = zeros(0, 0);

scale = 1;

config = @(steady_state, power_scale, processor_thickness) ...
  Utils.configStream(...
    'verbose', 0, ...
    'deadline_ratio', 1, ...
    'leakage', 0, ...
    'steady_state', steady_state, ...
    'power_scale', power_scale, ...
    [ 'hotspot t_chip ', num2str(processor_thickness) ]);

fprintf('%15s%15s%15s%15s%15s%15s%15s%15s\n', 'Thickness, mm', 'Conductance', ...
  'Capacitance', 'Power scale', 'T min, C', 'T mean, C', 'T max, C', 'RMSE');

for i = 1:length(processorThickness)
  thickness = processorThickness(i);

  [ G, C ] = Optima.get_coefficients(...
    system, floorplan, hotspot, params, config(0, scale, thickness));

  if i == 1
    temperature1 = Optima.solve(...
      system, floorplan, hotspot, params, config(0, scale, thickness));
    meanT = mean(mean(temperature1));
    standard = meanT;
  else
    while true
      temperature1 = Optima.solve(...
        system, floorplan, hotspot, params, config(0, scale, thickness));

      meanT = mean(mean(temperature1));
      if meanT >= standard, break; end;

      scale = scale + 0.01;
    end
  end

  meanT = meanT - Constants.degreeKelvin;
  minT = min(min(temperature1)) - Constants.degreeKelvin;
  maxT = max(max(temperature1)) - Constants.degreeKelvin;

  temperature2 = Optima.solve(...
    system, floorplan, hotspot, params, config(1, scale, thickness));

  stepCount = size(temperature1, 1);
  error = sqrt(sum((temperature2 - temperature1).^ 2) / stepCount);

  meanError = mean(error);

  fprintf('%15.4f%15.4f%15.2e%15.2f%15.2f%15.2f%15.2f%15.2f\n', ...
    thickness * 1e3, G(1, 1), C(1), scale, minT, meanT, maxT, meanError);

  Error(end + 1, :) = error;
  Scale(end + 1) = scale;
end

processorThickness = processorThickness * 1e3;

figure;

Utils.drawLines('Temperature RMSE', ...
  'Thickness of the chip, mm', 'RMSE', ...
  processorThickness, Error, [], 'Line', '--', 'Color', 'k');

x = processorThickness(1):step:processorThickness(end);
y = interp1(processorThickness, mean(Error, 2), x, 'cubic');
line(x, y, 'Color', Constants.roundRobinColors{1}, 'LineWidth', 2);

figure;

Utils.drawLines('Power Scale', ...
  'Thickness of the chip, mm', 'Scale', ...
  processorThickness, transpose(Scale), [], 'Marker', 'o');
