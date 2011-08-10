classdef Constants < handle
  properties (Constant)
    % Working directory, everything goes here
    workingDirectory = '../build';

    % HotSpot version
    hotspotVersion = '5.0';

    % TGFF
    graphLabels = { 'TASK_GRAPH' };
    peLabels = { 'PE' };

    % Visualization
    roundRobinColors = { 'r', 'g', 'b', 'm', 'y', 'c' };

    % Sampling interval
    samplingInterval = 1e-3; % s

    % Ambient temperature
    ambientTemperature = 318.15; % K

    % Peak threshold of local minima and maxima (for the cycle counting)
    peakThreshold = 1.0; % K

    % Kelvin to Celsius
    degreeKelvin = 273.15;
  end
end
