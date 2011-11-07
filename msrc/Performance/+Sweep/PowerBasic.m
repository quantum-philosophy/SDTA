classdef PowerBasic < Sweep.Basic
  properties (SetAccess = protected)
    nominalMaxPower
    nominalStepCount
  end

  methods
    function sweep = PowerBasic(test, varargin)
      sweep = sweep@Sweep.Basic(test, varargin{:});

      config = sweep.config;

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', '');

      power = Optima.get_power( ...
        config.system, config.floorplan, config.hotspot, ...
        config.params, param_line);

      sweep.nominalMaxPower = max(max(power));
      sweep.nominalStepCount = size(power, 1);
    end
  end
end
