classdef Optima < handle
  methods (Static)
    [ conductance, capacitance ] = ...
      get_coefficients(system, floorplan, hotspot, params, param_line);
    [ temperature, power, time ] = ...
      solve(system, floorplan, hotspot, params, param_line);
    [ reference_temperature, refeference_time, power, iterations, temperature, time ] = ...
      verify(system, floorplan, hotspot, params, param_line, max_iterations, tolerance);
  end
end
