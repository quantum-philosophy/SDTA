clear all;
clc;

name = '001_030';

tgff = Utils.path([ name, '.tgffopt' ]);
system = Utils.path([ name, '.sys' ]);
hotspot = Utils.path('hotspot.config');
floorplan = Utils.path([ name, '_temp.flp' ]);
params = Utils.path('parameters.config');

processorCount = Utils.readParameter(tgff, 'table_cnt');
nodeCount = 4 * processorCount + 12;

processorArea = [ 1e-6, 2e-6, 4e-6, 8e-6, 16e-6, 32e-6, 64e-6, 128e-6 ];

fprintf('%15s', 'Area, mm^2');

for i = 1:processorCount
  fprintf('%15s', 't die, ms');
end

fprintf('\n');

for i = 1:length(processorArea)
  area = processorArea(i);

  Utils.generateFloorplan(floorplan, processorCount, area);
  [ G, C ] = Optima.get_coefficients(system, floorplan, hotspot, params);

  G = G(1:processorCount, 1:processorCount);
  C = C(1:processorCount);

  t = C ./ diag(G);

  fprintf('%15d', area * 1e6);
  fprintf('%15f', t);
  fprintf('\n');
end
