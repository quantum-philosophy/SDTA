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
    function o = Optima(name, processorCount)
      o.name = name;
      o.tgffopt = Utils.path([ name, '.tgffopt' ]);
      o.tgff = Utils.path([ name, '.tgff' ]);
      o.system = Utils.path([ name, '_system.config' ]);
      o.hotspot = Utils.path([ name, '_hotspot.config' ]);
      o.floorplan = Utils.path([ name, '_floorplan.config' ]);
      o.params = Utils.path([ name, '_params.config' ]);

      if nargin > 1
        o.processorCount = processorCount;
      else
        o.processorCount = Utils.readParameter(o.tgffopt, 'table_cnt');
      end

      o.samplingInterval = Utils.readParameter(o.hotspot, '-sampling_intvl');
      o.ambientTemperature = Utils.readParameter(o.hotspot, '-ambient');
    end

    function changeArea(o, area)
      o.processorArea = area;
      o.floorplan = Utils.temp([ o.name, '_floorplan.config' ]);
      Utils.generateFloorplan(o.floorplan, o.processorCount, area);
    end

    function changeProcessorCountAndArea(o, count, area)
      o.processorCount = count;

      original = o.tgffopt;
      o.tgffopt = Utils.path([ o.name, '_temp.tgffopt' ]);
      o.tgff = Utils.path([ o.name, '_temp.tgff' ]);
      o.system = Utils.path([ o.name, '_system.config_temp' ]);

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
      o.hotspot = Utils.temp([ o.name, '_hotspot.config' ]);

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

    function [ sinkSide, spreaderSide, dieSide, sinkThickness ] = ...
      changePackage(o, spreaderSide, sinkSide, sinkThickness)

      if isempty(o.processorArea), error('The processor area is unknown'); end

      % Die
      dieArea = o.processorArea * o.processorCount;
      dieSide = sqrt(dieArea);

      if nargin < 2, spreaderSide = o.spreaderRatio * dieSide; end
      if nargin < 3, sinkSide = o.sinkRatio * spreaderSide; end
      if nargin < 4, sinkThickness = o.sinkThicknessRatio * sinkSide; end

      original = o.hotspot;
      o.hotspot = Utils.temp([ o.name, '_hotspot.config' ]);

      Utils.writeParameter(original, o.hotspot, '-s_sink', sinkSide);
      Utils.writeParameter(o.hotspot, o.hotspot, '-t_sink', sinkThickness);
      Utils.writeParameter(o.hotspot, o.hotspot, '-s_spreader', spreaderSide);
    end

    function standardChip(o)
      o.changeArea(4e-6);
    end

    function [ sinkSide, spreaderSide, dieSide, sinkThickness ] = standardPackage(o)
      [ sinkSide, spreaderSide, dieSide, sinkThickness ] = ...
        o.changePackage(20e-3, 30e-3, 15e-3);
    end

    function changeSamplingInterval(o, samplingInterval)
      o.samplingInterval = samplingInterval;

      original = o.hotspot;
      o.hotspot = Utils.temp([ o.name, '_hotspot.config' ]);

      Utils.writeParameter(original, o.hotspot, '-sampling_intvl', samplingInterval);
    end

    function graph = taskGraph(o)
      tgff = TestCase.TGFF(o.tgff);

      graph = tgff.graphs{1};
      pes = tgff.pes;

      equalLoad = Utils.readParameter(o.params, 'equal_load');

      if equalLoad == 1
        for i = 2:length(pes)
          pes{i}.equalLoadTo(pes{1});
        end
      end

      LS.mapEarliestAndSchedule(graph, pes);

      deadlineRatio = Utils.readParameter(o.params, 'deadline_ratio');

      graph.assignDeadline(deadlineRatio * graph.duration);
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

    [ temperature, power, time, iterations ] = ...
      verify(system, floorplan, hotspot, params, param_line, reference);

    lifetime = predict(temperature, sampling_interval);
  end
end
