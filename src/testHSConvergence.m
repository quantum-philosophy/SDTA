% Test: Let us look how long time it takes to HotSpot to converge

clear all;
clc;
rng(0);

repeat = 20;
ts = 1e-5;
desiredTime = 1; %s

name = 'simple';
floorplan = Utils.path([ name, '.flp' ]);
testCase = Utils.path([ name, '.tgff' ]);
config = Utils.path('hotspot.config');

% Parse tasks graphs
Utils.startTimer('Parse the test case');
tgff = TestCase.TGFF(testCase);
Utils.stopTimer();

% Take just first graph for the moment
graph = tgff.graphs{1};
pes = tgff.pes;
cores = length(pes);

% Thermal model
hotspot = HotSpot(floorplan, config);

% Dummy mapping
mapping = Utils.generateEvenMapping(cores, length(graph.tasks));
graph.assignMapping(pes, mapping);

% LS scheduling
Utils.startTimer('Scheduling in time across all the cores');
LS.schedule(graph);
Utils.stopTimer();

graph.fitTime(desiredTime);

graph.inspect();

% Power profile
Utils.startTimer('Generate a dynamic power profile');
powerProfile = Power.calculateDynamicProfile(graph, ts);
Utils.stopTimer();

steps = size(powerProfile, 1);

fprintf('Number of steps: %d\n', steps);
fprintf('Total simulation time: %.3f s\n', steps * ts);

% Draw a bit
% Utils.drawSimulation(graph, powerProfile);

% Original HotSpot
powerFile = sprintf('cores_%d_steps_%d.ptrace', cores, steps);
powerFile = Utils.path(powerFile);

Utils.startTimer('Dump the power profile');
Utils.dumpPowerProfile(powerFile, powerProfile);
Utils.stopTimer();

start = tic;
[ T, t ] = hotspot.solvePlainOriginal(powerFile, steps, repeat, ts, false);
t = toc(start);

totalSteps = size(T, 1);
am = Constants.ambientTemperature - Constants.degreeKelvin; % C
maxT = max(max(T)); % C

figure;

% Curves
subplot(2, 1, 1);
x = ((1:totalSteps) - 1) * ts;
Utils.drawLines(...
  sprintf('HotSpot, %d cores, %d x %d steps (%.2f s)',...
  cores, steps, repeat, t), 'Time, s', 'Temperature, C', x, T);

set(gca, 'YLim', [ 0 (maxT + 20) ]);

x = ([1, totalSteps] - 1) * ts;

% Ambient temperature
line(x, [ am am ],  'Color', 'k', 'Line', '--');

% Maximal temperature
line(x, [ maxT maxT ],  'Color', 'k', 'Line', '--');

% Analysis
fprintf('%5s%15s%15s\n', '#', 'norm(dT)', 'max(abs(dT))');
for i = 1:(repeat - 1)
  x = i * steps * ts;
  line([ x x ], [ 0 (maxT + 20) ], 'Color', 'k', 'Line', '--');

  cb = (i - 1) * steps + 1;
  ce = i * steps;
  nb = i * steps + 1;
  ne = (i + 1) * steps;

  dT = T(cb:ce, :) - T(nb:ne, :);

  fprintf('%5d%15f%15f\n', i + 1, norm(dT), max(max(abs(dT))));
end

% Condensed Equation
start = tic;
Tce = hotspot.solveCondensedEquation(powerProfile, ts) - Constants.degreeKelvin;
tce = toc(start);

subplot(2, 1, 2);
x = ((1:steps) - 1) * ts;
Utils.drawLines(...
  sprintf('Condensed Equation (%.2f s)', tce), ...
  'Time, s', 'Temperature, C', x, Tce);

maxT = max(max(Tce)); % C

dT = Tce - T(nb:ne, :);
fprintf('%5s%15f%15f\n', 'CE', norm(dT), max(max(abs(dT))));

set(gca, 'YLim', [ 0 (maxT + 20) ]);

x = ([1, steps] - 1) * ts;

% Ambient temperature
line(x, [ am am ],  'Color', 'k', 'Line', '--');

% Maximal temperature
line(x, [ maxT maxT ],  'Color', 'k', 'Line', '--');
