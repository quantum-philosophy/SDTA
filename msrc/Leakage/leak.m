setup;

config = Optima('001');

param_line = @(leakage) ...
  Utils.configStream(...
    'verbose', 0, ...
    'leakage', leakage, ...
    'solution', 'condensed_equation');

samplingInterval = config.samplingInterval;

dynamicPowerProfile = Optima.get_power( ...
  config.system, config.floorplan, config.hotspot, config.params, '');

[ stepCount, processorCount ] = size(dynamicPowerProfile);

maxPower = max(dynamicPowerProfile);
totalTime = stepCount * samplingInterval;

fprintf('%10s%15s\n', 'Core', 'Max power, W');
for i = 1:processorCount
  fprintf('%10d%15.2f\n', i, maxPower(i));
end
fprintf('%10s%15.2f\n', 'total', max(sum(dynamicPowerProfile, 2)));
fprintf('\n');

fprintf('Simulation time: %.2f s\n', totalTime);
fprintf('Number of steps: %d\n', stepCount);
fprintf('Number of cores: %d\n', processorCount);

figure;

x = ((1:stepCount) - 1) * samplingInterval;

% Dynamic power profile
subplot(2, 3, 1);
Utils.drawLines('Dynamic Power Profile', 'Time, s', 'Power, W', ...
  x, dynamicPowerProfile);
line(x, sum(dynamicPowerProfile, 2), 'Color', 'k', 'Line', '--');

[ T, t ] = Optima.solve_power(config.system, config.floorplan, config.hotspot, ...
  config.params, param_line(''), dynamicPowerProfile);

T = T - Constants.degreeKelvin;

[ Tleak, tleak, totalPowerProfile ] = Optima.solve_power(config.system, ...
  config.floorplan, config.hotspot, config.params, param_line('exponential'), ...
  dynamicPowerProfile);

Tleak = Tleak - Constants.degreeKelvin;

leakagePowerProfile = totalPowerProfile - dynamicPowerProfile;

Edynamic = sum(sum(dynamicPowerProfile * samplingInterval));
Etotal = sum(sum(totalPowerProfile * samplingInterval));
Eleakage = sum(sum(leakagePowerProfile * samplingInterval));

fprintf('Energy without leakage:  %.2f J\n', Edynamic);
fprintf('Energy with leakage:     %.2f J\n', Etotal);
fprintf('Energy due to leakage:   %.2f J\n', Eleakage);

fprintf('Leakage part in total:   %.2f %%\n', Eleakage / Etotal * 100);
fprintf('Leakage part in dynamic: %.2f %%\n', Eleakage / Edynamic * 100);

% Static power profile
subplot(2, 3, 2);
Utils.drawLines('Static Power Profile', 'Time, s', 'Power, W', ...
  x, leakagePowerProfile);
line(x, sum(leakagePowerProfile, 2), 'Color', 'k', 'Line', '--');

% Total power profile
subplot(2, 3, 3);
Utils.drawLines('Total Power Profile', 'Time, s', 'Power, W', ...
  x, totalPowerProfile);
line(x, sum(totalPowerProfile, 2), 'Color', 'k', 'Line', '--');

% Temperature profile without leakage
subplot(2, 3, 4);
Utils.drawLines('Temperature without leakage', 'Time, s', 'Temperature, C', x, T);

% Temperature difference
subplot(2, 3, 5);
Utils.drawLines('Leakage temperature', 'Time, s', 'Temperature, C', x, Tleak - T);

% Temperature profile
subplot(2, 3, 6);
Utils.drawLines('Temperature with leakage', ...
  'Time, s', 'Temperature, C', x, Tleak);

% Ambient line
x = ([1, stepCount] - 1) * samplingInterval;
am = config.ambientTemperature - Constants.degreeKelvin;
subplot(2, 3, 4);
line(x, [ am, am ],  'Color', 'k', 'Line', '--');
subplot(2, 3, 6);
line(x, [ am, am ],  'Color', 'k', 'Line', '--');

% Balance axes
subplot(2, 3, 3);
YLim = get(gca, 'YLim');
YLim(1) = 0;
set(gca, 'YLim', YLim);
subplot(2, 3, 1);
set(gca, 'YLim', YLim);
subplot(2, 3, 2);
set(gca, 'YLim', YLim);

subplot(2, 3, 6);
YLim = get(gca, 'YLim');
set(gca, 'YLim', YLim);
subplot(2, 3, 4);
set(gca, 'YLim', YLim);

subplot(2, 3, 1);
set(gca, 'XLim', [ 0 totalTime ]);
subplot(2, 3, 2);
set(gca, 'XLim', [ 0 totalTime ]);
subplot(2, 3, 3);
set(gca, 'XLim', [ 0 totalTime ]);
subplot(2, 3, 4);
set(gca, 'XLim', [ 0 totalTime ]);
subplot(2, 3, 5);
set(gca, 'XLim', [ 0 totalTime ]);
subplot(2, 3, 6);
set(gca, 'XLim', [ 0 totalTime ]);
