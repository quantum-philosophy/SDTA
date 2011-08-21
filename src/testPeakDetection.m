% Test: Find local minima and maxima on the temperature curves

clear all;
clc;
rng(0);

[ graph, hotspot, powerProfile ] = setup('test_cases/test_case_4_60');
[ steps, cores ] = size(powerProfile);

x = ((1:steps) - 1) * Constants.samplingInterval;

T = hotspot.solveCondensedEquation(powerProfile);
mttf = Lifetime.predictAndDraw(T)
