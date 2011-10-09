classdef Power < Sweep.Basic
  properties (SetAccess = private)
    processorPower
    nominalPower
  end

  methods
    function sweep = Power(test, processorPower, processorArea)
      sweep = sweep@Sweep.Basic(test);
      sweep.variable = 'Power, W';

      sweep.processorPower = processorPower;
      sweep.nominalPower = [];

      if nargin > 2
        sweep.floorplan = Utils.path([ test, '_tmp.flp' ]);
        Utils.generateFloorplan(sweep.floorplan, sweep.processorCount, processorArea);
      end
    end
  end

  methods (Access = protected)
    function result = continueStep(sweep, i)
      if i > length(sweep.processorPower), result = false;
      else result = true;
      end
    end

    function config = setupStep(sweep, i)
      if isempty(sweep.nominalPower)
        powerScale = 1;
      else
        powerScale = sweep.processorPower(i) / sweep.nominalPower;
      end

      config = { 'power_scale', powerScale };
    end

    function [ value, retake ] = valueStep(sweep, i, Tce, Tss, power)
      value = max(sum(power, 2));
      retake = false;

      if isempty(sweep.nominalPower)
        sweep.nominalPower = value;
        retake = true;
      end
    end
  end
end
