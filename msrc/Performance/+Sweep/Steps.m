classdef Steps < Sweep.PowerBasic
  properties (SetAccess = private)
    stepCount
  end

  methods
    function sweep = Steps(test, stepCount, varargin)
      sweep = sweep@Sweep.PowerBasic(test, varargin{:});
      sweep.variable = 'Application Period, s';
      sweep.stepCount = stepCount;
    end
  end

  methods (Access = protected)
    function result = continueStep(sweep, i)
      if i > length(sweep.stepCount), result = false;
      else result = true;
      end
    end

    function [ value, config ] = setupStep(sweep, i)
      o = sweep.config;

      stepCount = sweep.stepCount(i);
      value = stepCount * o.samplingInterval;

      timeScale = stepCount / sweep.nominalStepCount;

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'time_scale', timeScale, ...
          'leakage', '');

      sweep.power = Optima.get_power(o.system, o.floorplan, o.hotspot, ...
        o.params, param_line);

      config = { ...
          'time_scale', timeScale ...
      };
    end
  end
end
