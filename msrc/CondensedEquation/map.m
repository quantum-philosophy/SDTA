setup;

powerScale = 1.5;
config = Optima('002_006');

param_line = Utils.configStream( ...
  'verbose', 0, ...
  'leakage', 1, ...
  'solution', 'condensed_equation');

deadlineRatio = Utils.readParameter(config.params, 'deadline_ratio');


graph = TestCase.Graph(1, 'Application', 0);

for i = 1:6
  graph.addTask([ 'task', num2str(i) ], i);
end

graph.addLink([], 'task1', 'task2', 0);
graph.addLink([], 'task1', 'task3', 0);
graph.addLink([], 'task2', 'task4', 0);
graph.addLink([], 'task4', 'task5', 0);
graph.addLink([], 'task4', 'task6', 0);

cores = {};
for i = 1:2
  core = TestCase.Processor(i, [ 'core', num2str(i) ], i, 1e9, 1, 2e5);

  core.addType(1, 1.5e-8, 4.0e+7);
  core.addType(2, 1.5e-8, 3.0e+7);
  core.addType(3, 1.5e-8, 7.0e+7);
  core.addType(4, 1.5e-8, 4.0e+7);
  core.addType(5, 1.5e-8, 5.0e+7);
  core.addType(6, 1.5e-8, 4.0e+7);

  cores{end + 1} = core;
end

graph.assignDeadline(0.24);

figure;

% Before optimization
LS.mapEarliestAndSchedule(graph, cores);

graph.inspect();

subplot(2, 2, 1);
graph.draw(false, false);

power = Power.calculateDynamicProfile(graph) * powerScale;
T = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, power) - Constants.degreeKelvin;

subplot(2, 2, 2);
Utils.drawTemperature(T, 'SSDTC');

mn1 = min(min(T));
mx1 = max(max(T));

% After optimization
chromosome = [ 0, 1, 2, 5, 3, 4, 0, 1, 1, 0, 1, 0 ];
chromosome = chromosome + 1;
chromosomeLength = length(chromosome);
priority = chromosome(1:(chromosomeLength / 2));
mapping = chromosome((chromosomeLength / 2 + 1):end);

graph.assignMapping(cores, mapping);
LS.schedule(graph, priority);

graph.inspect();

subplot(2, 2, 3);
graph.draw(false, false);

power = Power.calculateDynamicProfile(graph) * powerScale;
T = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, power) - Constants.degreeKelvin;

mn2 = min(min(T));
mx2 = max(max(T));

YLim = [ -0.5 + min(mn1, mn2), 0.5 + max(mx1, mx2) ];

subplot(2, 2, 2);
set(gca, 'YLim', YLim);

subplot(2, 2, 4);
Utils.drawTemperature(T, 'SSDTC');
set(gca, 'YLim', YLim);
