% Test: Just one execution of the Condensed Equation method

clear all;
clc;
rng(0);

[ hotspot, profile, cores, steps ] = setup('test_cases/test_case_4_60');

figure;

% The Condensed Equation Method
Utils.startTimer('Solve with the condensed equation method');
T = hotspot.solveCondensedEquation(profile) ;
t = Utils.stopTimer();
T = T - Constants.degreeKelvin;
x = ((1:steps) - 1) * Constants.samplingInterval;
Utils.drawLines(sprintf('Condensed Equation (%.3f s)', t), ...
  'Time, s', 'Temperature, C', x, T);
