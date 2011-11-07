classdef Cores < Sweep.PowerBasic
  properties (Constant)
    processorArea = 4e-6;
    stepCount = 100;
  end

  properties (SetAccess = private)
    power
    processorCount
  end

  methods
    function sweep = Cores(test, processorCount, varargin)
      sweep = sweep@Sweep.PowerBasic(test, varargin{:});
      sweep.variable = 'Number of Cores';
      sweep.processorCount = processorCount;

      sweep.config.changeProcessorCountAndArea( ...
        max(max(processorCount)), sweep.processorArea);
      sweep.config.scalePackage();
    end
  end

  methods (Access = protected)
    function result = continueStep(sweep, i)
      if i > length(sweep.processorCount), result = false;
      else result = true;
      end
    end

    function [ value, config ] = setupStep(sweep, i)
      processorCount = sweep.processorCount(i);
      value = sweep.processorCount(i);

      o = sweep.config;

      o.changeProcessorCountAndArea(processorCount, sweep.processorArea);

      sweep.hotspot = Hotspot(o.floorplan, ...
        o.hotspot, sweep.hotspot_line);

      sweep.power = rand(sweep.stepCount, processorCount) * sweep.nominalMaxPower;

      config = {};
    end

    function [ T, time ] = optimaSolveOnAverage(sweep, param_line)
      config = sweep.config;
      total = 0;

      for i = 1:sweep.tryCount
        [ T, t ] = Optima.solve_power( ...
          config.system, config.floorplan, config.hotspot, ...
          config.params, param_line, sweep.power);
        total = total + t;
      end

      time = total / sweep.tryCount;
    end

    function [ T, time ] = matlabOnAverage(sweep, param_line, method)
      if nargin < 3, method = 'band'; end

      if strcmp(method, 'band')
        n = 1;
      else
        n = sweep.tryCount;
      end

      total = 0;

      for i = 1:n
        [ T, t ] = sweep.hotspot.solve(sweep.power, method);
        total = total + t;
      end

      time = total / n;
    end
  end
end
