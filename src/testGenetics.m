clear all;
clc;
rng(0);

name = 'test_cases/test_case_4_60';

testCase = Utils.path([ name, '.tgff' ]);
floorplan = Utils.path([ name, '.flp' ]);
config = Utils.path('hotspot.config');

tgff = TestCase.TGFF(testCase);
graph = tgff.graphs{1};
processors = tgff.pes;

mapping = Utils.generateEvenMapping(length(processors), length(graph.tasks));
graph.assignMapping(processors, mapping);

LS.schedule(graph);
graph.assignDeadline(Constants.deadlineFactor * graph.duration);

dynamicPowerProfile = Power.calculateDynamicProfile(graph);
[ steps, cores ] = size(dynamicPowerProfile);

systemConfig = Utils.compactTaskGraph(graph, processors);
systemConfig.type = systemConfig.type - 1;

hotspotConfig = struct('floorplan', floorplan, 'config', config, ...
  'ambient', Constants.ambientTemperature, ...
  'init_temp', Constants.ambientTemperature, ...
  'sampling_intvl', Constants.samplingInterval);

Utils.startTimer('Solve with the GLSA');
Genetics.optimizeAging(hotspotConfig, systemConfig);
Utils.stopTimer();

switch 2
case 0 % Power, leakage, and temperature
  hotspot = HotSpot(floorplan, config);

  Utils.startTimer('Solve with the CE');
  [ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage(...
    dynamicPowerProfile, systemConfig.voltage, systemConfig.ngate, 0.01, 10);
  Utils.stopTimer();

  load('dynamic_power.mat');
  load('temperature.mat');
  load('total_power.mat');

  T = T - Constants.degreeKelvin;
  temperature = temperature - Constants.degreeKelvin;

  figure;

  x = ((1:steps) - 1) * Constants.samplingInterval;

  subplot(3, 3, 1);
  Utils.drawLines('Pdyn1', 'Time, s', 'P, W', x, dynamicPowerProfile);

  subplot(3, 3, 2);
  Utils.drawLines('Pdyn2', 'Time, s', 'P, W', x, dynamic_power);

  subplot(3, 3, 3);
  error = abs(dynamicPowerProfile - dynamic_power);
  Utils.drawLines('dPdyn', 'Time, s', 'dP, W', x, error);

  subplot(3, 3, 4);
  Utils.drawLines('T1', 'Time, s', 'T, C', x, T);

  subplot(3, 3, 5);
  Utils.drawLines('T2', 'Time, s', 'T, C', x, temperature);

  subplot(3, 3, 6);
  error = abs(T - temperature);
  Utils.drawLines('dT', 'Time, s', 'dT, C', x, error);

  subplot(3, 3, 7);
  Utils.drawLines('Ptot1', 'Time, s', 'P, W', x, totalPowerProfile);

  subplot(3, 3, 8);
  Utils.drawLines('Ptot2', 'Time, s', 'P, W', x, total_power);

  subplot(3, 3, 9);
  error = abs(totalPowerProfile - total_power);
  Utils.drawLines('dPtot', 'Time, s', 'dP, W', x, error);

case 1 % Reliability
  hotspot = HotSpot(floorplan, config);

  Utils.startTimer('Solve with the CE');
  [ T, it, totalPowerProfile ] = hotspot.solveCondensedEquationWithLeakage(...
    dynamicPowerProfile, systemConfig.voltage, systemConfig.ngate, 0.01, 10);
  Utils.stopTimer();

  % figure;
  % x = ((1:steps) - 1) * Constants.samplingInterval;
  % Utils.drawLines('T', 'Time, s', 'T, C', x, T);

  for i = 1:cores
    T0 = T(:, i);

    fprintf('Core #%d\n', i);
    [ damage, maxp, minp, cycles ] = Lifetime.calculateDamage(T0);

    I = sort([ maxp(:, 1); minp(:, 1) ]);
    T0 = T0(I);

    fprintf('  Peaks: %d\n', length(I));
    fprintf('    ');

    for j = 1:length(I)
      fprintf('[ %d %.2f ] ', I(j), T0(j));
    end

    fprintf('\n');

    fprintf(' Cycles: %d\n', length(cycles));
    fprintf(' Damage: %f\n', damage);
  end

  mttf = Lifetime.predict(T);
  fprintf('Lifetime: %.2f\n', min(mttf));
end
