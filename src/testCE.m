% Test: Just one execution of the Condensed Equation method

clear all;
clc;
rng(0);

[ graph, hotspot, powerProfile ] = setup('test_cases/test_case_4_60');

Utils.startTimer('Solve with the CE');
T = hotspot.solveCondensedEquation(powerProfile) ;
t = Utils.stopTimer();

Utils.drawSimulation(graph, powerProfile, T);
