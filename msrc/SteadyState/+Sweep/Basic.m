classdef Basic < handle
  properties (SetAccess = protected)
    config

    title
    variable

    values
    error
  end

  methods
    function sweep = Basic(test)
      config = Optima(test);
      sweep.config = config;

      sweep.title = 'Steady-State Error';
      sweep.variable = 'Variable';
    end

    function perform(sweep)
      fprintf('%15s%15s%15s%15s%15s%15s\n', ...
        'Parameter', 'P max, W', 'Tmin, C', 'Tmax, C', 'RMSE', 'NRMSE');

      sweep.values = zeros(0, 0);
      sweep.error = zeros(0);

      i = 1;

      while sweep.continueStep(i)
        while true
          [ Tce, Tss, power ] = sweep.makeStep(i);
          [ value, retake ] = sweep.valueStep(i, Tce, Tss, power);
          if ~retake, break; end
        end

        sweep.values(end + 1) = value;

        Pmax = max(sum(power, 2));

        Tmin = min(min(Tce)) - Constants.degreeKelvin;
        Tmax = max(max(Tce)) - Constants.degreeKelvin;

        rmse = Utils.RMSE(Tce, Tss);
        nrmse = Utils.NRMSE(Tce, Tss);

        fprintf('%15s%15.2f%15.2f%15.2f%15.2f%15.2f\n', ...
          num2str(value), Pmax, Tmin, Tmax, rmse, nrmse);

        sweep.error(end + 1) = nrmse * 100;

        i = i + 1;
      end
    end

    function draw(sweep, createFigure)
      if nargin < 2 || createFigure, figure; end

      values = sweep.values;
      error = sweep.error;

      Utils.drawLines(sweep.title, sweep.variable, 'Normalized RMSE, %', ...
        values, error, [], 'Line', '--', 'Color', 'k');

      step = (max(values) - min(values)) / (10 * length(values));

      x = values(1):step:values(end);
      y = interp1(values, mean(error, 2), x, 'cubic');

      line(x, y, 'Color', Constants.roundRobinColors{1}, 'LineWidth', 2);
    end
  end

  methods (Abstract, Access = protected)
    result = continueStep(i)
    config = setupStep(i)
    [ value, retake ] = valueStep(i, Tce, Tss, power)
  end

  methods (Access = private)
    function [ Tce, Tss, power ] = makeStep(sweep, i)
      params = sweep.setupStep(i);

      config = sweep.config;

      param_line = Utils.configStream(...
        'verbose', 0, ...
        'deadline_ratio', 1, ...
        'solution', 'condensed_equation', ...
        'leakage', '', params{:});

      [ Tce, power ] = Optima.solve(...
        config.system, config.floorplan, config.hotspot, config.params, param_line);

      param_line = Utils.configStream(...
        'verbose', 0, ...
        'deadline_ratio', 1, ...
        'solution', 'precise_steady_state', ...
        'leakage', '', params{:});

      Tss = Optima.solve(...
        config.system, config.floorplan, config.hotspot, config.params, param_line);
    end
  end
end
