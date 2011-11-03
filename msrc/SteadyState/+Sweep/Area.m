classdef Area < Sweep.Basic
  properties (SetAccess = private)
    totalTime = 1;

    convectionResistance = 0.1;
    powerDensity = 40e4;

    spreaderSide = 20e-3;
    sinkSide = 30e-3;
    sinkThickness = 10e-3;

    processorArea
    maximalPower

    nominalTime
    nominalPower
  end

  methods
    function sweep = Area(test, processorArea, convectionResistance, totalTime)

      sweep = sweep@Sweep.Basic(test);

      if nargin >= 3, sweep.convectionResistance = convectionResistance; end
      if nargin >= 4, sweep.totalTime = totalTime; end

      if sweep.config.processorCount == 1
        sweep.variable = 'Die area, mm^2';
      else
        sweep.variable = 'Processor area, mm^2';
      end

      sweep.processorArea = processorArea;
      sweep.maximalPower = processorArea * sweep.powerDensity;

      config = sweep.config;

      param_line = Utils.configStream(...
        'deadline_ratio', 1, ...
        'leakage', '');

      power = Optima.get_power(config.system, config.floorplan, ...
        config.hotspot, config.params, param_line);

      sweep.nominalTime = size(power, 1) * config.samplingInterval;
      sweep.nominalPower = max(max(power));
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
      power = sweep.maximalPower(i);

      sweep.config.changeArea(area);
      sweep.config.changePackage(sweep.spreaderSide, ...
        sweep.sinkSide, sweep.sinkThickness);

      timeScale = sweep.totalTime / sweep.nominalTime;
      powerScale = power / sweep.nominalPower;

      config = { ...
        'time_scale', timeScale, ...
        'power_scale', powerScale, ...
        'hotspot', [ 'r_convec ', num2str(sweep.convectionResistance) ] ...
      };
    end

    function [ value, retake ] = valueStep(sweep, i, Tce, Tss, power)
      value = sweep.processorArea(i) * 1e6;
      retake = false;
    end
  end
end
