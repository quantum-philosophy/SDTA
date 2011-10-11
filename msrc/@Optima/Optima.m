classdef Optima < handle
  properties (SetAccess = private)
    name
    tgffopt
    tgff
    system
    hotspot
    floorplan
    params

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
      o.floorplan = Utils.path([ o.name, '_temp.flp' ]);
      Utils.generateFloorplan(o.floorplan, o.processorCount, area);
    end
  end

  methods (Static)
    [ conductance, capacitance, inversed_capacitance ] = ...
      get_coefficients(floorplan, hotspot, hotspot_line);
    [ power ] = ...
      get_power(system, floorplan, hotspot, params, param_line);
    [ temperature, power, time ] = ...
      solve(system, floorplan, hotspot, params, param_line);
    [ reference_temperature, refeference_time, power, iterations, temperature, time ] = ...
      verify(system, floorplan, hotspot, params, param_line, max_iterations, tolerance);
  end
end
