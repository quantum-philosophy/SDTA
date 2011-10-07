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

fprintf('%15s%15s', 'Area, mm^2', 'Power scale');

for i = 1:processorCount
  fprintf('%15s', 'T min, K');
end

for i = 1:processorCount
  fprintf('%15s', 'T max, K');
end

for i = 1:processorCount
  fprintf('%15s', 'T avg, K');
end

for i = 1:processorCount
  fprintf('%15s', 'Variance');
end

fprintf('\n');

Variance = zeros(0, processorCount);
Scale = zeros(0, 0);

scale = 1;

for i = 1:length(processorArea)
  area = processorArea(i);

  Utils.generateFloorplan(floorplan, processorCount, area);

  if i == 1
    temperature1 = Optima.solve(system, floorplan, hotspot, params, ...
      sprintf('leakage 0 \n steady_state 0 \n power_scale %f', scale));
    standard = mean(mean(temperature1));
  else
    while true
      temperature1 = Optima.solve(system, floorplan, hotspot, params, ...
        sprintf('leakage 0 \n steady_state 0 \n power_scale %f', scale));

      average = mean(mean(temperature1));
      if average >= standard, break; end;

      scale = scale + 0.01;
    end
  end

  temperature2 = Optima.solve(system, floorplan, hotspot, params, ...
    sprintf('leakage 0 \n steady_state 1 \n power_scale %f', scale));

  stepCount = size(temperature1, 1);
  difference = temperature2 - temperature1;
  variance = sum(difference .^ 2) / (stepCount - 1);

  fprintf('%15d%15.2f', area * 1e6, scale);
  fprintf('%15.2f', min(temperature1));
  fprintf('%15.2f', max(temperature1));
  fprintf('%15.2f', mean(temperature1));
  fprintf('%15.2f', variance);
  fprintf('\n');

  Variance(end + 1, :) = variance;
  Scale(end + 1) = scale;
end

figure;

Utils.drawLines('Variance', 'Processor area, mm^2', 'Variance', ...
  processorArea * 1e6, Variance, [], 'Marker', 'o');

figure;

Utils.drawLines('Power scale', 'Processor area, mm^2', 'Scale', ...
  processorArea * 1e6, transpose(Scale), [], 'Marker', 'o');
