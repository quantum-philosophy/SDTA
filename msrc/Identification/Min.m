setup;

fprintf('On-line Thermal Aware Dynamic Voltage Scaling for Energy Optimization with Frequency/Temperature Dependency Consideration\n');
fprintf('\n');

A = 0.007^2;

Nc = [ 2.85, 1.0, 4.3 ] * 1e6;
Ceff = [ 1.0e-9, 0.9e-10, 1.5e-8 ];

V = [ 1.8, 1.7, 1.6 ];
f = [ 717.8, 658.8, 600.1 ] * 1e6;
E = [ 0.063, 0.017, 0.228 ];

t = Nc ./ f;
Ptot = E ./ t;
Density = Ptot ./ (A * 1e4);
Pdyn = Ceff .* f .* V.^2;

fprintf('%15s%15s%15s%15s%15s%15s%15s\n', ...
  'V, V', 'f, Mhz', 't, ms', 'E, J', 'P tot, W', 'D, W/cm^2', 'P dyn, W');

for i = 1:3
  fprintf('%15.1f%15.1f%15.2f%15.3f%15.3f%15.3f%15.3f\n', ...
    V(i), f(i) * 1e-6, t(i) * 1e3, E(i), Ptot(i), Density(i), Pdyn(i));
end

fprintf('\n');
fprintf('%15.2f\n', sum(t) * 1e3);

fprintf('\n');
fprintf('\n');

config = Optima('#min', 1);

spreaderRatio = 37.5 / 9;

config.changeArea(A);
[ sinkSide, spreaderSide, dieSide, sinkThickness ] = ...
  config.scalePackage(spreaderRatio);

config.changeSamplingInterval(1e-4);

fprintf('Die side: %.2f mm\n', dieSide * 1e3);
fprintf('Spreader side: %.2f mm\n', spreaderSide * 1e3);
fprintf('Sink side: %.2f mm\n', sinkSide * 1e3);
fprintf('Sink thickness: %.2f mm\n', sinkThickness * 1e3);

Pdynamic = Optima.get_power(config.system, config.floorplan, ...
  config.hotspot, config.params, '');

fprintf('P dynamic max: %.2f W\n', max(max(Pdynamic)));

param_line = Utils.configStream(...
  'solution', 'condensed_equation', ...
  'hotspot', 'r_convec 0.1', ...
  'verbose', 0, ...
  'leakage', 'exponential');

[ T, time, Ptotal ] = Optima.solve_power(config.system, config.floorplan, ...
    config.hotspot, config.params, param_line, Pdynamic);

fprintf('P total max: %.2f W\n', max(max(Ptotal)));

Utils.drawTemperature(T - Constants.degreeKelvin, ...
  'Temperature', config.samplingInterval);
