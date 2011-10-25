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

  core.addType(1, 1e-8, 2.0e+8);
  core.addType(2, 1e-8, 1.5e+8);
  core.addType(3, 1e-8, 3.5e+8);
  core.addType(4, 1e-8, 2.0e+8);
  core.addType(5, 1e-8, 2.0e+8);
  core.addType(6, 1e-8, 2.0e+8);

  cores{end + 1} = core;
end

figure;

% Before optimization
LS.mapEarliestAndSchedule(graph, cores);
deadline = deadlineRatio * graph.duration;
graph.assignDeadline(deadline);

graph.inspect();

subplot(2, 2, 1);
graph.draw(false, false);

power = Power.calculateDynamicProfile(graph) * powerScale;
T = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, power) - Constants.degreeKelvin;

subplot(2, 2, 2);
Utils.drawTemperature(T, 'SSDTC');

% After optimization
chromosome = [ 0, 1, 2, 5, 3, 4, 0, 1, 1, 0, 1, 0 ];
chromosome = chromosome + 1;
chromosomeLength = length(chromosome);
priority = chromosome(1:(chromosomeLength / 2));
mapping = chromosome((chromosomeLength / 2 + 1):end);

graph.assignMapping(cores, mapping);
LS.schedule(graph, priority);
graph.assignDeadline(deadline);

graph.inspect();

subplot(2, 2, 3);
graph.draw(false, false);

power = Power.calculateDynamicProfile(graph) * powerScale;
T = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, power) - Constants.degreeKelvin;

subplot(2, 2, 4);
Utils.drawTemperature(T, 'SSDTC');
