classdef Steps < Sweep.PowerBasic
  properties (SetAccess = private)
    stepCount
  end

  methods
    function sweep = Steps(test, stepCount)
      sweep = sweep@Sweep.PowerBasic(test);
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

      sweep.power = Power.generateRandomProfile( ...
        o.processorCount, sweep.stepCount(i), o.processorCount * sweep.powerPerCore);

      config = {};
    end

    function [ value, retake ] = valueStep(sweep, i, T, t)
      value = sweep.stepCount(i) * sweep.hotspot.samplingInterval;
      retake = false;
    end
  end
end
