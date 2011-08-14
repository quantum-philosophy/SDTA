% Test: Just to see how the temperature depends on the power

clear all;
clc;
rng(0);

name = 'simple';

cores = 1;
dieSize = 81e-6; % m^2
totalTime = 100; % s
floorplan = Utils.path([ name, '.flp' ]);
config = Utils.path('hotspot.config');

steps = floor(totalTime / Constants.samplingInterval);
Utils.generateFloorplan(floorplan, cores, dieSize);

hotspot = HotSpot(floorplan, config);

maxPower = 30:10:200;
T = zeros(0);

for power = maxPower
  powerProfile = Power.generateConstantProfile(cores, steps, power);
  T0 = hotspot.solveCondensedEquation(powerProfile);
  T(end+1, 1:size(T0, 2)) = mean(T0) - Constants.degreeKelvin;
  fprintf('Maximal power %d W, average temperature %.2f\n', power, T(end));
end

plot(maxPower, T, 'Color', 'r');
line(maxPower, T, 'LineStyle', 'none', 'Marker', 'o');
title('Tavg(Pmax)');
xlabel('Pmax, W');
ylabel('T, C');
