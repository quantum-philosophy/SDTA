classdef PowerBasic < Sweep.Basic
  properties (SetAccess = protected)
    nominalMaxPower
    nominalStepCount
    power
  end

  methods
    function sweep = PowerBasic(test, varargin)
      sweep = sweep@Sweep.Basic(test, varargin{:});

      config = sweep.config;

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', '');

      config.standardChip();
      config.standardPackage();

      power = Optima.get_power( ...
        config.system, config.floorplan, config.hotspot, ...
        config.params, param_line);

      sweep.nominalMaxPower = max(max(power));
      sweep.nominalStepCount = size(power, 1);
    end
  end

  methods (Access = protected)
    function [ T, time ] = optimaSolveOnAverage(sweep, param_line, Tref)
      config = sweep.config;

      for i = 1:sweep.tryCount
        [ T, t ] = Optima.solve_power( ...
          config.system, config.floorplan, config.hotspot, ...
          config.params, param_line, sweep.power);

        if i == 1
          total = t;
        else
          total = total + t;
        end
      end

      time = total / sweep.tryCount;
    end

    function [ T, time ] = optimaVerifyOnAverage(sweep, param_line, Tref)
      config = sweep.config;

      n = sweep.tryCount;

      for i = 1:n
        [ T, t, it ] = Optima.verify_power( ...
          config.system, config.floorplan, config.hotspot, ...
          config.params, param_line, sweep.power, Tref);

        if i == 1
          total = t;
        else
          total = total + t;
        end
      end

      time = total / n;
      time(end + 1) = it / 1000;
    end

    function [ T, time ] = matlabOnAverage(sweep, param_line, method, Tref)
      if nargin < 3, method = 'band'; end

      if strcmp(method, 'band') || strcmp(method, 'umf')
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

      time = [ time, 0, sweep.hotspot.decompositionTime, sweep.hotspot.preparationTime ];
    end
  end
end
