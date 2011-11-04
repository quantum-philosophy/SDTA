setup;

powerScale = 3;
config = Optima('002_006');

config.changeArea(16e-6);
config.changeSamplingInterval(1e-4);

param_line = Utils.configStream( ...
  'verbose', 0, ...
  'leakage', 'exponential', ...
  'solution', 'condensed_equation');

graph = TestCase.Graph(1, 'Application', 0);

for i = 1:4
  graph.addTask([ 'task', num2str(i) ], i);
end

graph.addLink([], 'task1', 'task2', 0);
graph.addLink([], 'task1', 'task3', 0);
graph.addLink([], 'task2', 'task4', 0);

cores = {};

core = TestCase.Processor(1, 'core1', 1, 1e9, 1, 2e5);
core.addType(1, 1.5e-8, 12.0e+6);
core.addType(2, 1.5e-8, 18.0e+6);
core.addType(3, 1.5e-8, 10.0e+6);
core.addType(4, 1.5e-8, 12.0e+6);
cores{end + 1} = core;

core = TestCase.Processor(2, 'core2', 2, 1e9, 1, 2e5);
core.addType(1, 1.5e-8, 14.0e+6);
core.addType(2, 1.5e-8, 20.0e+6);
core.addType(3, 1.5e-8, 12.0e+6);
core.addType(4, 1.5e-8, 14.0e+6);
cores{end + 1} = core;

graph.assignDeadline(0.050);

figure;

% Before optimization
priority = [ 1, 2, 3, 4 ];
mapping = [ 1, 2, 1, 1 ];

graph.assignMapping(cores, mapping);
LS.schedule(graph, priority);

graph.inspect();

subplot(2, 2, 1);
graph.draw(false, false);
title('');

power = Power.calculateDynamicProfile(graph) * powerScale;
T1 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, power) - Constants.degreeKelvin;

subplot(2, 2, 2);
Utils.drawTemperature(T1, []);

mn1 = min(min(T1));
mx1 = max(max(T1));

% After optimization
priority = [ 1, 2, 3, 4 ];
mapping = [ 1, 2, 1, 2 ];

graph.assignMapping(cores, mapping);
LS.schedule(graph, priority);

graph.inspect();

subplot(2, 2, 3);
graph.draw(false, false);
title('');

power = Power.calculateDynamicProfile(graph) * powerScale;
T2 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, power) - Constants.degreeKelvin;

mn2 = min(min(T2));
mx2 = max(max(T2));

YLim = [ -2 + min(mn1, mn2), 2 + max(mx1, mx2) ];

subplot(2, 2, 2);
set(gca, 'YLim', YLim);

subplot(2, 2, 4);
Utils.drawTemperature(T2, []);
set(gca, 'YLim', YLim);

Lifetime.predictMultipleAndDraw(T1 + Constants.degreeKelvin)

Lifetime.predictMultipleAndDraw(T2 + Constants.degreeKelvin)
