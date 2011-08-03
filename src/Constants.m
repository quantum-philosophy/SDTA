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

    % Power model
    K1 = 0.063;
    K2 = 0.153;
    K3 = 5.38e-7;
    K4 = 1.83;
    K5 = 4.19;
    K6 = 5.26e-12;
    Ld = 10;
    Ij = 4.80e-10;
    Vth1 = 0.244;
    alpha = 1;
  end
end
