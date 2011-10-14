classdef Constants < handle
  properties (Constant)
    % Working directory, everything goes here
    workingDirectory = [ Constants.thisDirectory, '/../build/test' ];

    % HotSpot version
    hotspotVersion = '5.0';

    % TGFF
    graphLabels = { 'TASK_GRAPH' };
    peLabels = { 'PE' };

    % How to determine the deadline? Let us add a time gap after
    % the actual duration of the task graph.
    deadlineFactor = 1.1;

    % Visualization
    roundRobinColors = { ...
      [ 87/255, 181/255, 232/255 ], ...
      [ 230/255, 158/255, 0 ], ...
      [ 0, 158/255, 115/255 ], ...
      'r', 'm', 'y', 'c' };
    roundRobinMarkers = { 'o', 'x', 'v', 's', '^', '+' };

    % Kelvin to Celsius
    degreeKelvin = 273.15; % K

    % Ambient temperature
    ambientTemperature = 27 + Constants.degreeKelvin; % K

    % Sampling interval
    samplingInterval = 1e-3; % s

    % Temperature runaway
    temperatureRunaway = 120 + Constants.degreeKelvin; % K
  end

  methods (Static)
    function result = thisDirectory
      filename = mfilename('fullpath');
      attrs = regexp(filename, '^(.*)/[^/]+$', 'tokens');
      result = attrs{1}{1};
    end
  end
end
