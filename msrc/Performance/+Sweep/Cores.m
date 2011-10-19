classdef Cores < Sweep.PowerBasic
  properties (Constant)
    processorArea = 4e-6;
    stepCount = 100;
    maxPower = 80;
  end

  properties (SetAccess = private)
    processorCount
  end

  methods
    function sweep = Cores(test, processorCount)
      sweep = sweep@Sweep.PowerBasic(test);
      sweep.variable = 'Number of Processing Units';
      sweep.processorCount = processorCount;
    end
  end

  methods (Access = protected)
    function result = continueStep(sweep, i)
      if i > length(sweep.processorCount), result = false;
      else result = true;
      end
    end

    function config = setupStep(sweep, i)
      processorCount = sweep.processorCount(i);

      o = sweep.config;

      o.changeProcessorCountAndArea(processorCount, sweep.processorArea);
      o.scalePackage();

      sweep.hotspot = Hotspot(o.floorplan, ...
        o.hotspot, sweep.hotspot_line);

      sweep.power = Power.generateRandomProfile( ...
        processorCount, sweep.stepCount, sweep.maxPower);

      config = {};
    end

    function [ value, retake ] = valueStep(sweep, i, T, t)
      value = sweep.processorCount(i);
      retake = false;
    end
  end
end
