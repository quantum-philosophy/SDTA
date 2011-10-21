classdef PowerBasic < Sweep.Basic
  properties (SetAccess = protected)
    power
  end

  methods
    function sweep = PowerBasic(test, varargin)
      sweep = sweep@Sweep.Basic(test, varargin{:});
    end
  end

  methods (Access = protected)
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

    function [ T, time ] = optimaVerifyOnAverage(sweep, param_line)
      config = sweep.config;
      total = 0;

      for i = 1:sweep.tryCount
        [ dummy, dummy, dummy, T, t ] = Optima.verify_power( ...
          config.system, config.floorplan, config.hotspot, ...
          config.params, param_line, sweep.power, sweep.maxIterations, sweep.tolerance);
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
