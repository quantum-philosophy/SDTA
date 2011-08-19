% Test: Let us look how long time it takes to HotSpot to converge

clear all;
clc;
rng(0);

repeat = 20;
desiredTime = 1; % s

name = 'test_cases/test_case_4_30';
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

% graph.fitTime(desiredTime);

graph.inspect();

% Power profile
Utils.startTimer('Generate a dynamic power profile');
powerProfile = Power.calculateDynamicProfile(graph);
Utils.stopTimer();

steps = size(powerProfile, 1);

fprintf('Number of steps: %d\n', steps);
fprintf('Total simulation time: %.3f s\n', steps * Constants.samplingInterval);

% Draw a bit
% Utils.drawSimulation(graph, powerProfile);

figure;

am = Constants.ambientTemperature - Constants.degreeKelvin; % C

% Condensed Equation
Utils.startTimer('Solve with the CE');
Tce = hotspot.solveCondensedEquation(powerProfile) - Constants.degreeKelvin;
tce = Utils.stopTimer();

Tmince = min(min(Tce));
Tmaxce = max(max(Tce));

fprintf('Minimal temperature: %.2f C\n', Tmince);
fprintf('Maximal temperature: %.2f C\n', Tmaxce);

subplot(2, 1, 2);
x = ((1:steps) - 1) * Constants.samplingInterval;
Utils.drawLines(sprintf('Condensed Equation (%.2f s)', tce), ...
  'Time, s', 'Temperature, C', x, Tce);

maxT = max(max(Tce)); % C

set(gca, 'XLim', [ 0 x(end) ]);
set(gca, 'YLim', [ 0 (maxT + 20) ]);

x = ([1, steps] - 1) * Constants.samplingInterval;
line(x, [ am am ],  'Color', 'k', 'Line', '--');
line(x, [ maxT maxT ],  'Color', 'k', 'Line', '--');

% Original HotSpot
powerFile = sprintf('cores_%d_steps_%d.ptrace', cores, steps);
powerFile = Utils.path(powerFile);

Utils.startTimer('Dump the power profile');
Utils.dumpPowerProfile(powerFile, powerProfile);
Utils.stopTimer();

Utils.startTimer('Solve with HotSpot');
[ T, t ] = hotspot.solvePlainOriginal(powerFile, steps, repeat, false);
Utils.stopTimer();

fprintf('Speed up: %.2f\n', t / tce);

totalSteps = size(T, 1);

subplot(2, 1, 1);
x = ((1:totalSteps) - 1) * Constants.samplingInterval;
Utils.drawLines(...
  sprintf('HotSpot, %d cores, %d x %d steps (%.2f s)',...
  cores, steps, repeat, t), 'Time, s', 'Temperature, C', x, T);

maxT = max(max(T)); % C

set(gca, 'XLim', [ 0 x(end) ]);
set(gca, 'YLim', [ 0 (maxT + 20) ]);

x = ([1, totalSteps] - 1) * Constants.samplingInterval;
line(x, [ am am ],  'Color', 'k', 'Line', '--');
line(x, [ maxT maxT ],  'Color', 'k', 'Line', '--');

% Analysis
fprintf('%5s%15s%15s%15s%15s%15s\n', ...
  '#', 'norm(dT)', 'max(dT)', 'Bottom', 'Top', 'Error');
for i = 1:(repeat - 1)
  x = i * steps * Constants.samplingInterval;
  line([ x x ], [ 0 (maxT + 20) ], 'Color', 'k', 'Line', '--');

  pb = (i - 1) * steps + 1;
  pe = i * steps;
  nb = i * steps + 1;
  ne = (i + 1) * steps;

  Tprev = T(pb:pe, :);
  Tnext = T(nb:ne, :);
  dT = Tprev - Tnext;

  diff = max(max(abs(dT)));
  bottom = Tmince - min(min(Tnext));
  top = Tmaxce - max(max(Tnext));
  error = max(max(Utils.calcError(Tnext, Tce)));

  fprintf('%5d%15.2f%15.2f%15.2f%15.2f%15.2f\n', i + 1, ...
    norm(dT), diff, bottom, top, error);
end
