% Test: Testdrive for the power leakage

clear all;
clc;

name = 'test_cases/test_case_4_60';

floorplan = Utils.path([ name, '.flp' ]);
testCase = Utils.path([ name, '.tgff' ]);
config = Utils.path('hotspot.config');

% Parse tasks graphs. Note, we are only interested in PEs here.
Utils.startTimer('Obtain the description of PEs');
tgff = TestCase.TGFF(testCase);
Utils.stopTimer();
pes = tgff.pes;

for pe = pes, pe{1}.inspect(); end

tol = 0.01; % K
maxit = 10;
maxPower = 150; % W
simulationTime = 100; % s
steps = simulationTime / Constants.samplingInterval;
cores = length(pes);

fprintf('Maximal power:   %d W\n', maxPower);
fprintf('Simulation time: %d s\n', simulationTime);
fprintf('Number of steps: %d\n', steps);
fprintf('Number of cores: %d\n', cores);

% Thermal model
hotspot = HotSpot(floorplan, config);

% Random power profile
dynamicPowerProfile = 0.2 + rand(steps, cores);
for i = 1:steps
  multiplier = maxPower / sum(dynamicPowerProfile(i, :));
  dynamicPowerProfile(i, :) = multiplier * dynamicPowerProfile(i, :);
end

figure;

x = ((1:steps) - 1) * Constants.samplingInterval;

% Dynamic power profile
subplot(3, 1, 1);
Utils.drawLines(...
  sprintf('Dynamic Power Profile (total %d W)', maxPower), ...
  'Time, s', 'Power, W', ...
  x, dynamicPowerProfile);
line(x, sum(dynamicPowerProfile, 2), 'Color', 'k', 'Line', '--');

T = ones(size(dynamicPowerProfile)) * Constants.ambientTemperature;

for i = 1:maxit
  % Static power profile
  staticPowerProfile = Power.calculateStaticProfile(pes, T);

  % The Condensed Equation Method
  Utils.startTimer();
  nextT = hotspot.solveCondensedEquation(dynamicPowerProfile + staticPowerProfile);
  t = Utils.stopTimer();

  error = max(max(abs(T - nextT)));

  fprintf('Iteration %d, solved in %.2f s, error %0.2f C\n', i, t, error);

  T = nextT;
  if error < tol, break; end
end

% Static power profile
subplot(3, 1, 2);
Utils.drawLines('Static Power Profile', 'Time, s', 'Power, W', ...
  x, staticPowerProfile);
line(x, sum(staticPowerProfile, 2), 'Color', 'k', 'Line', '--');

T = T - Constants.degreeKelvin;

% Temperature profile
subplot(3, 1, 3);
Utils.drawLines(...
  sprintf('Temperature Profile (%d iterations)', i), ...
  'Time, s', 'Temperature, C', x, T);

x = ([1, steps] - 1) * Constants.samplingInterval;

am = Constants.ambientTemperature - Constants.degreeKelvin;
line(x, [ am, am ],  'Color', 'k', 'Line', '--');
