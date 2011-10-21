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

    function config = setupStep(sweep, i)
      param_line = Utils.configStream('verbose', 0);

      o = sweep.config;

      sweep.power = ones(sweep.stepCount(i), o.processorCount);

      config = {};
    end

    function [ value, retake ] = valueStep(sweep, i, T, t)
      value = sweep.stepCount(i) * sweep.hotspot.samplingInterval;
      retake = false;
    end
  end
end
