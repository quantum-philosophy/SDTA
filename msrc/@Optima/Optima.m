classdef Optima < handle
  properties (Constant)
    spreaderRatio = 21 / 3.99; % Relative to the die
    sinkRatio = sqrt(25 * 28) / 21; % Relative to the spreader
    sinkThicknessRatio = 15 / sqrt(25 * 28); % Relative to the sink
  end

  properties (SetAccess = private)
    name
    tgffopt
    tgff
    system
    hotspot
    floorplan
    params

    processorArea
    processorCount
    samplingInterval
    ambientTemperature
  end

  methods
    function o = Optima(name)
      o.name = name;
      o.tgffopt = Utils.path([ name, '.tgffopt' ]);
      o.tgff = Utils.path([ name, '.tgff' ]);
      o.system = Utils.path([ name, '_system.config' ]);
      o.hotspot = Utils.path([ name, '_hotspot.config' ]);
      o.floorplan = Utils.path([ name, '_floorplan.config' ]);
      o.params = Utils.path([ name, '_params.config' ]);

      o.processorCount = Utils.readParameter(o.tgffopt, 'table_cnt');
      o.samplingInterval = Utils.readParameter(o.hotspot, '-sampling_intvl');
      o.ambientTemperature = Utils.readParameter(o.hotspot, '-ambient');
    end

    function changeArea(o, area)
      o.processorArea = area;
      o.floorplan = Utils.path([ o.name, '_temp.flp' ]);
      Utils.generateFloorplan(o.floorplan, o.processorCount, area);
    end

    function changeProcessorCountAndArea(o, count, area)
      o.processorCount = count;

      original = Utils.path([ o.name, '.tgffopt' ]);

      o.tgffopt = Utils.path([ o.name, '_temp.tgffopt' ]);
      o.tgff = Utils.path([ o.name, '_temp.tgff' ]);
      o.system = Utils.path([ o.name, '_temp.sys' ]);

      Utils.writeParameter(original, o.tgffopt, 'table_cnt', count);
      Utils.tgffopt(o.tgffopt, o.tgff, o.system);

      o.changeArea(area);
    end

    function [ sinkSide, spreaderSide, dieSide, sinkThickness ] = ...
      scalePackage(o, spreaderRatio, sinkRatio, sinkThicknessRatio)

      if isempty(o.processorArea), error('The processor area is unknown'); end

      if nargin < 2, spreaderRatio = o.spreaderRatio; end
      if nargin < 3, sinkRatio = o.sinkRatio; end
      if nargin < 4, sinkThicknessRatio = o.sinkThicknessRatio; end

      original = o.hotspot;
      o.hotspot = Utils.path([ o.name, '_hotspot_temp.config' ]);

      % Die
      dieArea = o.processorArea * o.processorCount;
      dieSide = sqrt(dieArea);
      % Spreader
      spreaderSide = spreaderRatio * dieSide;
      % Sink
      sinkSide = sinkRatio * spreaderSide;
      % Sink thickness
      sinkThickness = sinkThicknessRatio * sinkSide;

      Utils.writeParameter(original, o.hotspot, '-s_sink', sinkSide);
      Utils.writeParameter(o.hotspot, o.hotspot, '-t_sink', sinkThickness);
      Utils.writeParameter(o.hotspot, o.hotspot, '-s_spreader', spreaderSide);
    end

    function changeSamplingInterval(o, samplingInterval)
      o.samplingInterval = samplingInterval;

      original = o.hotspot;
      o.hotspot = Utils.path([ o.name, '_hotspot_temp.config' ]);

      Utils.writeParameter(original, o.hotspot, '-sampling_intvl', samplingInterval);
    end
  end

  methods (Static)
    [ conductance, capacitance, inversed_capacitance ] = ...
      get_coefficients(floorplan, hotspot, hotspot_line);
    [ power ] = ...
      get_power(system, floorplan, hotspot, params, param_line);

    [ temperature, power, time ] = ...
      solve(system, floorplan, hotspot, params, param_line);
    [ temperature, time, total_power ] = ...
      solve_power(system, floorplan, hotspot, params, param_line, power);
    [ intervals, temperature, power, time ] = ...
      solve_coarse(system, floorplan, hotspot, params, param_line);

    [ reference_temperature, refeference_time, power, iterations, temperature, time ] = ...
      verify(system, floorplan, hotspot, params, param_line, max_iterations, tolerance);
    [ reference_temperature, refeference_time, iterations, temperature, time ] = ...
      verify_power(system, floorplan, hotspot, params, param_line, power, max_iterations, tolerance);
  end
end
