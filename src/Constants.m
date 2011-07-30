classdef Constants < handle
  properties (Constant)
    % Working directory, everything goes here
    workingDirectory = '../build';

    % TGFF
    graphLabel = 'TASK_GRAPH';
    peLabel = 'PE';

    % Visualization
    roundRobinColors = { 'r', 'g', 'b', 'm', 'y', 'c' };

    % Sampling interval
    samplingInterval = 0.7e-3;

    % Ambient temperature
    ambientTemperature = 45.0;

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
