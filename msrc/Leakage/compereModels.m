setup;

config = Optima('016');

param_line = @(leakage) ...
  Utils.configStream('verbose', 0, 'leakage', leakage);

samplingInterval = config.samplingInterval;

Pdyn = Optima.get_power( ...
  config.system, config.floorplan, config.hotspot, config.params, '');

T = Optima.solve_power( ...
  config.system, config.floorplan, config.hotspot, ...
  config.params, param_line(''), Pdyn);

[ Texp, dummy, Ptot ] = Optima.solve_power( ...
  config.system, config.floorplan, config.hotspot, ...
  config.params, param_line('exponential'), Pdyn);

Tlin = Optima.solve_power(config.system, config.floorplan, config.hotspot, ...
  config.params, param_line('linear'), Pdyn);

Pleak = Ptot - Pdyn;

Edyn = sum(sum(Pdyn * samplingInterval));
Etot = sum(sum(Ptot * samplingInterval));
Eleak = sum(sum(Pleak * samplingInterval));

fprintf('Leakage part in total:   %.2f %%\n', Eleak / Etot * 100);
fprintf('Leakage part in dynamic: %.2f %%\n', Eleak / Edyn * 100);

x = ((1:size(Pdyn, 1)) - 1) * samplingInterval;

T = T - Constants.degreeKelvin;
Texp = Texp - Constants.degreeKelvin;
Tlin = Tlin - Constants.degreeKelvin;

figure;

subplot(3, 1, 1);
Utils.drawLines('Without leakage', 'Time, s', 'Temperature, C', x, T);

subplot(3, 1, 2);
Utils.drawLines('Exponential leakage', 'Time, s', 'Temperature, C', x, Texp);

subplot(3, 1, 3);
Utils.drawLines('Linear leakage', 'Time, s', 'Temperature, C', x, Tlin);

Utils.equalScales(3, 1, 1);

fprintf('NRMSE: %.4f\n', Utils.NRMSE(Texp, Tlin));
