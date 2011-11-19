setup;

config = Optima('004');

param_line = Utils.configStream('verbose', 0);

graph = config.taskGraph();
graph.draw();

P1 = Power.calculateDynamicProfile(graph, config.samplingInterval);

T1 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, P1);

chromosome = [ 0, 1, 2, 4, 3, 5, 6, 15, 7, 8, 12, 14, 17, 16, 9, 11, 21, 19, 13, 10, 18, 23, 24, 32, 20, 31, 29, 25, 37, 26, 38, 34, 33, 36, 35, 39, 27, 28, 30, 22, 0, 3, 3, 0, 2, 0, 3, 2, 3, 2, 0, 3, 2, 2, 0, 0, 0, 3, 2, 2, 3, 2, 2, 0, 3, 2, 0, 2, 2, 2, 0, 0, 3, 3, 2, 3, 3, 1, 2, 3 ] + 1;

taskCount = length(graph.tasks);
priority = chromosome(1:taskCount);
mapping = chromosome((taskCount + 1):end);
graph.assignMapping([], mapping);
LS.schedule(graph, priority);

graph.draw();

if graph.duration > graph.deadline
  fprintf('Deadline: %.2f\n', graph.deadline);
  fprintf('Duration: %.2f\n', graph.duration);
  error('The deadline is not met.');
end

P2 = Power.calculateDynamicProfile(graph, config.samplingInterval);

T2 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line, P2);

[ stepCount, processorCount ] = size(T1);

time = ((1:stepCount) - 1) * Constants.samplingInterval;

figure;

subplot(2, 1, 1);
Utils.drawLines('SSDTP before', 'Time, s', 'Temperature, C', ...
  time, T1 - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);

subplot(2, 1, 2);
Utils.drawLines('SSDTP after', 'Time, s', 'Temperature, C', ...
  time, T2 - Constants.degreeKelvin);
set(gca, 'XLim', [ 0 time(end) ]);

MTTF1 = min(Lifetime.predictMultipleAndDraw(T1, config.samplingInterval));
MTTF2 = min(Lifetime.predictMultipleAndDraw(T2, config.samplingInterval));

fprintf('MTTF1: %.2f\n', MTTF1);
fprintf('MTTF2: %.2f\n', MTTF2);
fprintf('Improvement: %.2f\n', MTTF2 / MTTF1);
