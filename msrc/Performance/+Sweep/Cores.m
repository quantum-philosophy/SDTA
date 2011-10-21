classdef Cores < Sweep.PowerBasic
  properties (Constant)
    processorArea = 4e-6;
    stepCount = 100;
  end

  properties (SetAccess = private)
    processorCount
  end

  methods
    function sweep = Cores(test, processorCount, varargin)
      sweep = sweep@Sweep.PowerBasic(test, varargin{:});
      sweep.variable = 'Number of Cores';
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

      sweep.power = ones(sweep.stepCount, processorCount);

      config = {};
    end

    function [ value, retake ] = valueStep(sweep, i, T, t)
      value = sweep.processorCount(i);
      retake = false;
    end
  end
end
