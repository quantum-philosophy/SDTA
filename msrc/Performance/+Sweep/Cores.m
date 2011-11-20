classdef Cores < Sweep.PowerBasic
  properties (Constant)
    totalTime = 0.5;
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

    function [ value, config ] = setupStep(sweep, i)
      processorCount = sweep.processorCount(i);
      value = processorCount;

      test = sprintf('%03d', processorCount);
      config = Optima(test);
      sweep.config = config;

      sweep.hotspot = Hotspot(config.floorplan, ...
        config.hotspot, sweep.hotspot_line);

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', '');

      power = Optima.get_power( ...
        config.system, config.floorplan, config.hotspot, ...
        config.params, param_line);

      timeScale = sweep.totalTime / (config.samplingInterval * size(power, 1));

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'time_scale', timeScale, ...
          'leakage', '');

      sweep.power = Optima.get_power( ...
        config.system, config.floorplan, config.hotspot, ...
        config.params, param_line);

      config = { 'time_scale', timeScale };
    end
  end
end
