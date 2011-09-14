% Test: Just one execution of the Condensed Equation method

clear all;
clc;
rng(0);

[ graph, hotspot, powerProfile ] = setup('test_cases/004_010');

graph.inspect();

Utils.startTimer('Solve with the CE');
T1 = hotspot.solveCondensedEquation(powerProfile) ;
Utils.stopTimer();

Utils.drawSimulation(graph, powerProfile, T1);

dummy = zeros(1, length(graph.pes));

Utils.startTimer('Solve with the CE (leakage)');
T2 = hotspot.solveCondensedEquationWithLeakage(...
  powerProfile, dummy, dummy, 0.01, 1);
Utils.stopTimer();

Utils.drawSimulation(graph, powerProfile, T2);

Utils.startTimer('Solve with in Matlab');
T3 = hotspot.solveNativeCondensedEquation(powerProfile);
Utils.stopTimer();

Utils.drawSimulation(graph, powerProfile, T3);

fprintf('Error T1 and T2: %f\n', abs(max(max(T2 - T1))));
fprintf('Error T1 and T3: %f\n', abs(max(max(T3 - T1))));
