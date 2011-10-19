classdef Optima < handle
  properties (Constant)
    spreaderRatio = (37.5 * 37.5) / 81;
    sinkRatio = 2 * Optima.spreaderRatio;
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
  end

  methods
    function o = Optima(name)
      o.name = name;
      o.tgffopt = Utils.path([ name, '.tgffopt' ]);
      o.tgff = Utils.path([ name, '.tgff' ]);
      o.system = Utils.path([ name, '.sys' ]);
      o.hotspot = Utils.path('hotspot.config');
      o.floorplan = Utils.path([ name, '.flp' ]);
      o.params = Utils.path('parameters.config');

      o.processorCount = Utils.readParameter(o.tgffopt, 'table_cnt');
      o.samplingInterval = Utils.readParameter(o.hotspot, '-sampling_intvl');
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

    function scalePackage(o)
      if isempty(o.processorArea), error('The processor area is unknown'); end

      original = Utils.path('hotspot.config');

      o.hotspot = Utils.path('hotspot_temp.config');

      % Die
      dieArea = o.processorArea * o.processorCount;
      dieSide = sqrt(dieArea);
      % Spreader
      spreaderSide = sqrt(o.spreaderRatio * dieArea);
      % Sink
      sinkSide = sqrt(o.sinkRatio * dieArea);

      Utils.writeParameter(original, o.hotspot, '-s_sink', sinkSide);
      Utils.writeParameter(o.hotspot, o.hotspot, '-s_spreader', spreaderSide);
    end
  end

  methods (Static)
    [ conductance, capacitance, inversed_capacitance ] = ...
      get_coefficients(floorplan, hotspot, hotspot_line);
    [ power ] = ...
      get_power(system, floorplan, hotspot, params, param_line);

    [ temperature, power, time ] = ...
      solve(system, floorplan, hotspot, params, param_line);
    [ temperature, time ] = ...
      solve_power(system, floorplan, hotspot, params, param_line, power);

    [ reference_temperature, refeference_time, power, iterations, temperature, time ] = ...
      verify(system, floorplan, hotspot, params, param_line, max_iterations, tolerance);
    [ reference_temperature, refeference_time, iterations, temperature, time ] = ...
      verify_power(system, floorplan, hotspot, params, param_line, power, max_iterations, tolerance);
  end
end
