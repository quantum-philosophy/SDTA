classdef Basic < handle
  properties (SetAccess = protected)
    tgffopt
    tgff
    system
    hotspot
    floorplan
    params

    processorCount

    title
    variable

    values
    Terror
  end

  methods
    function sweep = Basic(test)
      sweep.tgffopt = Utils.path([ test, '.tgffopt' ]);
      sweep.tgff = Utils.path([ test, '.tgff' ]);
      sweep.system = Utils.path([ test, '.sys' ]);
      sweep.hotspot = Utils.path('hotspot.config');
      sweep.floorplan = Utils.path([ test, '.flp' ]);
      sweep.params = Utils.path('parameters.config');

      sweep.processorCount = Utils.readParameter(sweep.tgffopt, 'table_cnt');
      sweep.title = 'Temperature RMSE';
      sweep.variable = 'Variable';
    end

    function perform(sweep)
      fprintf('%15s%15s%15s%15s%15s%15s%15s\n', ...
        'P max, W', 'Tce max, C', 'dT CE, C', 'Tss max, C', 'dT SS, C', ...
        'Ratio', 'RMSE');

      sweep.values = zeros(0, 0);
      sweep.Terror = zeros(0, sweep.processorCount);

      i = 1;

      while sweep.continueStep(i)
        while true
          [ Tce, Tss, power ] = sweep.makeStep(i);
          [ value, retake ] = sweep.valueStep(i, Tce, Tss, power);
          if ~retake, break; end
        end

        sweep.values(end + 1) = value;

        Pmax = max(sum(power, 2));

        TminCE = min(min(Tce)) - Constants.degreeKelvin;
        TavgCE = mean(mean(Tce)) - Constants.degreeKelvin;
        TmaxCE = max(max(Tce)) - Constants.degreeKelvin;
        dTCE = TmaxCE - TminCE;

        TminSS = min(min(Tss)) - Constants.degreeKelvin;
        TavgSS = mean(mean(Tss)) - Constants.degreeKelvin;
        TmaxSS = max(max(Tss)) - Constants.degreeKelvin;
        dTSS = TmaxSS - TminSS;

        ratio = dTSS / dTCE;

        stepCount = size(power, 1);
        error = sqrt(sum((Tce - Tss).^ 2) / stepCount);

        fprintf('%15.2f%15.2f%15.2f%15.2f%15.2f%15.2f%15.2f\n', ...
          Pmax, TmaxCE, dTCE, TmaxSS, dTSS, ratio, mean(error));

        sweep.Terror(end + 1, :) = error;

        i = i + 1;
      end
    end

    function draw(sweep)
      figure;

      values = sweep.values;
      Terror = sweep.Terror;

      Utils.drawLines(sweep.title, sweep.variable, 'RMSE', ...
        values, Terror, [], 'Line', '--', 'Color', 'k');

      step = (max(values) - min(values)) / (10 * length(values));

      x = values(1):step:values(end);
      y = interp1(values, mean(Terror, 2), x, 'cubic');

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
      config = sweep.setupStep(i);

      configCE = Utils.configStream(...
        'verbose', 0, ...
        'deadline_ratio', 1, ...
        'steady_state', 0, ...
        'leakage', 0, config{:});

      configSS = Utils.configStream(...
        'verbose', 0, ...
        'deadline_ratio', 1, ...
        'steady_state', 1, ...
        'leakage', 0, config{:});

      [ Tce, power ] = Optima.solve(...
        sweep.system, sweep.floorplan, sweep.hotspot, sweep.params, configCE);

      Tss = Optima.solve(...
        sweep.system, sweep.floorplan, sweep.hotspot, sweep.params, configSS);
    end
  end
end
