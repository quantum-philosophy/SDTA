setup;

powerScale = 3;
config = Optima('002_006');

samplingInterval = 1e-4;

config.changeArea(16e-6);
config.changeSamplingInterval(samplingInterval);

param_line = Utils.configStream( ...
  'verbose', 0, ...
  'leakage', 'exponential', ...
  'solution', 'condensed_equation');

graph = TestCase.Graph(1, 'Application', 0);

for i = 1:6
  graph.addTask([ 'task', num2str(i) ], i);
end

graph.addLink([], 'task1', 'task2', 0);
graph.addLink([], 'task1', 'task3', 0);
graph.addLink([], 'task1', 'task4', 0);
graph.addLink([], 'task3', 'task5', 0);
graph.addLink([], 'task5', 'task6', 0);

cores = {};

core = TestCase.Processor(1, 'core1', 1, 1e9, 1, 2e5);
core.addType(1, 1.4e-8, 10.0e+6);
core.addType(2, 0.2e-8, 10.0e+6);
core.addType(3, 1.0e-8, 15.0e+6);
core.addType(4, 1.5e-8, 10.0e+6);
core.addType(5, 1.5e-8, 15.0e+6);
core.addType(6, 1.5e-8, 10.0e+6);
cores{end + 1} = core;

core = TestCase.Processor(2, 'core2', 2, 1e9, 1, 2e5);
core.addType(1, 1.4e-8, 12.0e+6);
core.addType(2, 0.2e-8, 12.0e+6);
core.addType(3, 1.0e-8, 17.0e+6);
core.addType(4, 1.5e-8, 12.0e+6);
core.addType(5, 1.5e-8, 17.0e+6);
core.addType(6, 1.5e-8, 12.0e+6);
cores{end + 1} = core;

graph.assignDeadline(0.06);

figure;

% Before optimization
priority = [ 1, 2, 3, 4, 5, 6 ];
mapping = [ 1, 1, 2, 1, 2, 1 ];

graph.assignMapping(cores, mapping);
LS.schedule(graph, priority);

graph.inspect();

subplot(3, 2, 1);
graph.draw(false);
title('');

power = Power.calculateDynamicProfile(graph, samplingInterval) * powerScale;
T1 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, power);

subplot(3, 2, 2);
Utils.drawTemperature(T1 - Constants.degreeKelvin, [], samplingInterval);

% After optimization
priority = [ 1, 2, 3, 4, 5, 6 ];
mapping = [ 1, 1, 2, 1, 2, 2 ];

graph.assignMapping(cores, mapping);
LS.schedule(graph, priority);

graph.inspect();

subplot(3, 2, 3);
graph.draw(false);
title('');

power = Power.calculateDynamicProfile(graph, samplingInterval) * powerScale;
T2 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, power);

subplot(3, 2, 4);
Utils.drawTemperature(T2 - Constants.degreeKelvin, [], samplingInterval);

% After more optimization
priority = [ 1, 4, 3, 2, 5, 6 ];
mapping = [ 1, 1, 2, 1, 2, 2 ];

graph.assignMapping(cores, mapping);
LS.schedule(graph, priority);

graph.inspect();

subplot(3, 2, 5);
graph.draw(false);
title('');

power = Power.calculateDynamicProfile(graph, samplingInterval) * powerScale;
T3 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, power);

subplot(3, 2, 6);
Utils.drawTemperature(T3 - Constants.degreeKelvin, [], samplingInterval);

mn = min([ min(min(T1)), min(min(T2)), min(min(T3)) ]) - Constants.degreeKelvin;
mx = max([ max(max(T1)), max(max(T2)), max(max(T3)) ]) - Constants.degreeKelvin;

YLim = [ -2 + mn, 2 + mx ];

subplot(3, 2, 1);
xlabel('Time, s');
subplot(3, 2, 2);
set(gca, 'YLim', YLim);
xlabel('Time, s');
ylabel('Temperature, C');

subplot(3, 2, 3);
xlabel('Time, s');
subplot(3, 2, 4);
set(gca, 'YLim', YLim);
xlabel('Time, s');
ylabel('Temperature, C');

subplot(3, 2, 5);
xlabel('Time, s');
subplot(3, 2, 6);
set(gca, 'YLim', YLim);
xlabel('Time, s');
ylabel('Temperature, C');

m1 = Optima.predict(T1, samplingInterval);
m2 = Optima.predict(T2, samplingInterval);
m3 = Optima.predict(T3, samplingInterval);

fprintf('MTTF 1: %.2f\n', m1);
fprintf('MTTF 2: %.2f\n', m2);
fprintf('MTTF 3: %.2f\n', m3);

fprintf('Improvement 1: %.2f%%\n', (m2 / m1 - 1) * 100);
fprintf('Improvement 2: %.2f%%\n', (m3 / m1 - 1) * 100);
