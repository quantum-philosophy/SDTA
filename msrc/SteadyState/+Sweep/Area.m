classdef Area < Sweep.Basic
  properties (Constant)
    keepRatio = true;
    ratio = (37.5 * 37.5) / 81; % Heat sink area to die area
  end

  properties (SetAccess = private)
    processorArea
    powerScale
    nominalTemperature
  end

  methods
    function sweep = Area(test, processorArea)
      sweep = sweep@Sweep.Basic(test);
      if sweep.processorCount == 1
        sweep.variable = 'Area of the die, mm^2';
      else
        sweep.variable = 'Area of one processing element, mm^2';
      end
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
      area = sweep.processorArea(i);

      Utils.generateFloorplan(sweep.floorplan, ...
        sweep.processorCount, area);

      if sweep.keepRatio
        % Die
        dieArea = area * sweep.processorCount;
        dieSide = sqrt(dieArea);
        % Sink
        sinkSide = sqrt(sweep.ratio * dieArea);
        % Spreader
        spreaderSide = (dieSide + sinkSide) / 2;

        hotspot_line = sprintf('s_sink %.4f s_spreader %.4f', sinkSide, spreaderSide);

        config = { 'hotspot', hotspot_line, 'power_scale', sweep.powerScale };
      else
        config = { 'power_scale', sweep.powerScale };
      end
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
