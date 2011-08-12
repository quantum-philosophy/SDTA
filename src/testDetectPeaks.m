% Test: Find local minima and maxima on the temperature curves

clear all;
clc;
rng(0);

[ hotspot, profile, cores, steps ] = setup('test_cases/test_case_4_60');

figure;

x = ((1:steps) - 1) * Constants.samplingInterval;

T = hotspot.solveCondensedEquation(profile);
mttf = Lifetime.predictAndDraw(T)
