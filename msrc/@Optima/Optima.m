classdef Optima < handle
  methods (Static)
    [ temperature, power ] = solve(system, floorplan, hotspot, params, extra);
    [ conductance, capacitance ] = get_coefficients(system, floorplan, hotspot, params, extra);
  end
end
