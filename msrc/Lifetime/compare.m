setup;

config = Optima('001');

param_line = @(solution) ...
  Utils.configStream(...
    'verbose', 0, ...
    'solution', solution);

graph = config.taskGraph();

P1 = Power.calculateDynamicProfile(graph, config.samplingInterval);

T1 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line('condensed_equation'), P1);

P2 = Power.calculateDynamicProfile(graph, config.samplingInterval);

T2 = Optima.solve_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line('precise_steady_state'), P2);

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
