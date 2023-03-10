% Test: Testdrive for the power leakage

clear all;
clc;
rng(0);

tol = 0.01; % K
maxit = 10;

[ graph, hotspot, dynamicPowerProfile ] = setup('004_060');
[ steps, cores ] = size(dynamicPowerProfile);

maxPower = max(dynamicPowerProfile);
totalTime = steps * Constants.samplingInterval;

[ vdd, ngate ] = Utils.collectLeakageParams(graph);
pes = graph.pes;

fprintf('%10s%15s\n', 'Core', 'Max power, W');
for i = 1:cores
  fprintf('%10d%15.2f\n', i, maxPower(i));
end
fprintf('%10s%15.2f\n', 'total', max(sum(dynamicPowerProfile, 2)));
fprintf('\n');

fprintf('Simulation time: %.2f s\n', totalTime);
fprintf('Number of steps: %d\n', steps);
fprintf('Number of cores: %d\n', cores);

figure;

x = ((1:steps) - 1) * Constants.samplingInterval;

% Dynamic power profile
subplot(2, 3, 1);
Utils.drawLines('Dynamic Power Profile', 'Time, s', 'Power, W', ...
  x, dynamicPowerProfile);
line(x, sum(dynamicPowerProfile, 2), 'Color', 'k', 'Line', '--');

T = ones(size(dynamicPowerProfile)) * Constants.ambientTemperature;

startTic = tic;

for i = 1:maxit
  % Static power profile
  staticPowerProfile = Power.calculateStaticProfile(pes, T);

  % The Condensed Equation Method
  Utils.startTimer();
  Tnext = hotspot.solveCondensedEquation(dynamicPowerProfile + staticPowerProfile);
  t = Utils.stopTimer();

  Tmax = max(max(T));

  if Tmax > Constants.temperatureRunaway
    error('Detected a temperature runaway: %.f C\n', ...
      Tmax - Constants.degreeKelvin);
  end

  Terror = max(max(abs(T - Tnext)));

  fprintf('Iteration %d, solved in %.2f s, error %0.2f C\n', i, t, Terror);

  T = Tnext;
  if Terror < tol, break; end
end

t = toc(startTic);
T = T - Constants.degreeKelvin;

% The same, but in C++ only
Utils.startTimer();
[ Tcpp, icpp, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage(...
  dynamicPowerProfile, vdd, ngate, tol, maxit);
tcpp = Utils.stopTimer();
Tcpp = Tcpp - Constants.degreeKelvin;

fprintf('Solved in MatLab in %.2f s\n', t);
fprintf('Solved in C++    in %.2f s (%d iterations)\n', tcpp, icpp);
fprintf('Error: %f\n', max(max(abs(T - Tcpp))));

leakagePowerProfile = totalPowerProfile - dynamicPowerProfile;

Edynamic = sum(sum(dynamicPowerProfile * Constants.samplingInterval));
Etotal = sum(sum(totalPowerProfile * Constants.samplingInterval));
Eleakage = sum(sum(leakagePowerProfile * Constants.samplingInterval));

fprintf('Energy without leakage:    %.2f J\n', Edynamic);
fprintf('Energy with leakage:       %.2f J\n', Etotal);
fprintf('Energy because of leakage: %.2f J\n', Eleakage);

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

T = hotspot.solveCondensedEquation(dynamicPowerProfile) - Constants.degreeKelvin;

% Temperature profile without leakage
subplot(2, 3, 4);
Utils.drawLines('Temperature without leakage', 'Time, s', 'Temperature, C', x, T);

% Temperature difference
subplot(2, 3, 5);
Utils.drawLines('Leakage temperature', 'Time, s', 'Temperature, C', x, Tcpp - T);

% Temperature profile
subplot(2, 3, 6);
Utils.drawLines(...
  sprintf('Temperature with leakage', tcpp, icpp), ...
  'Time, s', 'Temperature, C', x, Tcpp);

% Ambient line
x = ([1, steps] - 1) * Constants.samplingInterval;
am = Constants.ambientTemperature - Constants.degreeKelvin;
subplot(2, 3, 4);
line(x, [ am, am ],  'Color', 'k', 'Line', '--');
subplot(2, 3, 6);
line(x, [ am, am ],  'Color', 'k', 'Line', '--');

% Balance axes
subplot(2, 3, 3);
YLim = get(gca, 'YLim');
YLim(1) = 0;
subplot(2, 3, 1);
set(gca, 'YLim', YLim);
subplot(2, 3, 2);
set(gca, 'YLim', YLim);

subplot(2, 3, 6);
YLim = get(gca, 'YLim');
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
