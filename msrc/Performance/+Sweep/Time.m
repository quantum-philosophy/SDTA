classdef Time < Sweep.Basic
  properties (SetAccess = private)
    timeScale
  end

  methods
    function sweep = Time(test, timeScale)
      sweep = sweep@Sweep.Basic(test);
      sweep.variable = 'Application Time, s';
      sweep.timeScale = timeScale;
    end
  end

  methods (Access = protected)
    function result = continueStep(sweep, i)
      if i > length(sweep.timeScale), result = false;
      else result = true;
      end
    end

    function config = setupStep(sweep, i)
      config = { 'time_scale', sweep.timeScale(i) };
    end

    function [ value, retake ] = valueStep(sweep, i, Tce, Tml, Ths, Tss)
      value = size(Tce, 1) * sweep.hotspot.samplingInterval;
      retake = false;
    end
  end
end
