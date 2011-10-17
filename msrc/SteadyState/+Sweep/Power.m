classdef Power < Sweep.Basic
  properties (Constant)
    keepRatio = true;
    ratio = (37.5 * 37.5) / 81; % Heat sink area to die area
  end

  properties (SetAccess = private)
    processorPower
    nominalPower
    hotspot_line
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

      if sweep.keepRatio
        % Die
        dieArea = processorArea * sweep.processorCount;
        dieSide = sqrt(dieArea);
        % Sink
        sinkSide = sqrt(sweep.ratio * dieArea);
        % Spreader
        spreaderSide = (dieSide + sinkSide) / 2;

        sweep.hotspot_line = ...
          sprintf('s_sink %.4f s_spreader %.4f', sinkSide, spreaderSide);
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

      if ~isempty(sweep.hotspot_line)
        config = { 'hotspot', sweep.hotspot_line, 'power_scale', powerScale };
      else
        config = { 'power_scale', powerScale };
      end
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
