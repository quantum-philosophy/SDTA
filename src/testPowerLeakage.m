% Test: Testdrive for the power leakage

clear all;
clc;
rng(0);

[ ssdtc, graph ] = setup('test_cases/test_case_4_60', true, false);

steps = ssdtc.stepCount;
cores = ssdtc.coreCount;

figure;

x = ((1:steps) - 1) * Constants.samplingInterval;

% Dynamic power profile
subplot(2, 2, 1);
profile = ssdtc.powerProfile;
Utils.drawLines('Dynamic Power Profile', 'Time, s', 'Power, W', x, profile);
line(x, sum(profile, 2), 'Color', 'k', 'Line', '--');

T = ones(size(profile)) * Constants.ambientTemperature;

% Static power profile
subplot(2, 2, 2);
profile = Power.calculateStaticProfile(graph, T);
Utils.drawLines('Static Power Profile (Tamb)', 'Time, s', 'Power, W', x, profile);
line(x, sum(profile, 2), 'Color', 'k', 'Line', '--');

% The Condensed Equation Method
subplot(2, 2, 3);
Utils.startTimer('Solve with the condensed equation method');
T = ssdtc.solveCondensedEquation();
Utils.stopTimer();
Utils.drawLines('Temperature Profile (T)', 'Time, s', 'Temperature, C', ...
  x, T - Constants.degreeKelvin);

% Static power profile
subplot(2, 2, 4);
profile = Power.calculateStaticProfile(graph, T);
Utils.drawLines('Static Power Profile (T)', 'Time, s', 'Power, W', x, profile);
line(x, sum(profile, 2), 'Color', 'k', 'Line', '--');
