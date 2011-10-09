classdef Area < Sweep.Basic
  properties (SetAccess = private)
    processorArea
    powerScale
    nominalTemperature
  end

  methods
    function sweep = Area(test, processorArea)
      sweep = sweep@Sweep.Basic(test);
      sweep.variable = 'Area of one core, mm^2';
      sweep.floorplan = Utils.path([ test, '_tmp.flp' ]);

      sweep.processorArea = processorArea;
      sweep.powerScale = 1;
      sweep.nominalTemperature = [];
    end
  end

  methods (Access = protected)
    function result = continueStep(sweep, i)
      if i > length(sweep.processorArea), result = false;
      else result = true;
      end
    end

    function config = setupStep(sweep, i)
      Utils.generateFloorplan(sweep.floorplan, ...
        sweep.processorCount, sweep.processorArea(i));

      config = { 'power_scale', sweep.powerScale };
    end

    function [ value, retake ] = valueStep(sweep, i, Tce, Tss, power)
      value = sweep.processorArea(i) * 1e6;
      retake = false;

      Tavg = mean(mean(Tce));

      if isempty(sweep.nominalTemperature)
        sweep.nominalTemperature = Tavg;
      end

      if Tavg < sweep.nominalTemperature
        sweep.powerScale = sweep.powerScale + 0.01;
        retake = true;
      end
    end
  end
end
