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
      value = sweep.stepCount(i) * sweep.hotspot.samplingInterval;
      config = { 'time_scale', sweep.stepCount(i) / sweep.nominalStepCount };
    end
  end
end
