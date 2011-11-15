setup;

config = Optima('002');

param_line = @(solution) ...
  Utils.configStream(...
    'verbose', 0, ...
    'leakage', 'linear', ...
    'solution', solution);

graph = config.taskGraph();
graph.draw();

P1 = Power.calculateDynamicProfile(graph, config.samplingInterval);

T1 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line('condensed_equation'), P1);

chromosome = [ 0, 1, 2, 7, 5, 9, 8, 6, 3, 4, 15, 12, 13, 14, 19, 18, 10, 11, 16, 17, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0 ] + 1;

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
  config.hotspot, config.params, param_line('condensed_equation'), P2);

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

fprintf('MTTF1: %.4f\n', MTTF1);
fprintf('MTTF2: %.4f\n', MTTF2);
