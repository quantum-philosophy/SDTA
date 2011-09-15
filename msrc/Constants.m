classdef Constants < handle
  properties (Constant)
    % Working directory, everything goes here
    workingDirectory = '../build/test';

    % HotSpot version
    hotspotVersion = '5.0';

    % TGFF
    graphLabels = { 'TASK_GRAPH' };
    peLabels = { 'PE' };

    % How to determine the deadline? Let us add a time gap after
    % the actual duration of the task graph.
    deadlineFactor = 1.1;

    % Visualization
    roundRobinColors = { 'r', 'g', 'b', 'm', 'y', 'c' };

    % Kelvin to Celsius
    degreeKelvin = 273.15; % K

    % Ambient temperature
    ambientTemperature = 27 + Constants.degreeKelvin; % K

    % Sampling interval
    samplingInterval = 1e-3; % s

    % Temperature runaway
    temperatureRunaway = 120 + Constants.degreeKelvin; % K
  end
end
