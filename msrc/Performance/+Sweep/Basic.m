classdef Basic < handle
  properties (Constant)
    max_iterations = 1;
    tolerance = 0;
    hotspot_line = '';

    tryCount = 10;

    includeSS = false;
  end

  properties (SetAccess = protected)
    tgffopt
    tgff
    system
    hotspot_config
    floorplan
    params

    title
    variable

    hotspot

    values
    times
  end

  methods
    function sweep = Basic(test)
      sweep.tgffopt = Utils.path([ test, '.tgffopt' ]);
      sweep.tgff = Utils.path([ test, '.tgff' ]);
      sweep.system = Utils.path([ test, '.sys' ]);
      sweep.hotspot_config = Utils.path('hotspot.config');
      sweep.floorplan = Utils.path([ test, '.flp' ]);
      sweep.params = Utils.path('parameters.config');

      sweep.title = 'Performance';
      sweep.variable = 'Variable';

      sweep.hotspot = Hotspot(sweep.floorplan, ...
        sweep.hotspot_config, sweep.hotspot_line);
    end

    function perform(sweep)
      fprintf('%15s%15s%15s%15s%15s%15s%15s\n', ...
        'CE, s', ...
        'UMF, s', 'Error(UMF)', ...
        'HS, s', 'Error(HS)', ...
        'SS, s', 'Error(SS)');

      sweep.values = zeros(0, 0);
      sweep.times = zeros(0, 4);

      i = 1;

      while sweep.continueStep(i)
        while true
          [ Tce, tce, Tml, tml, Ths, ths, Tss, tss ] = sweep.makeStep(i);
          [ value, retake ] = sweep.valueStep(i, Tce, Tml, Ths, Tss);
          if ~retake, break; end
        end

        sweep.values(end + 1) = value;

        sweep.times(end + 1, 1:4) = [ ths, tml, tce, tss ];

        Ehs = sweep.error(Tce, Ths);
        Eml = sweep.error(Tce, Tml);
        Ess = sweep.error(Tce, Tss);

        fprintf('%15.4f%15.4f%15.2e%15.4f%15.2e%15.4f%15.2e\n', ...
          tce, tml, mean(Eml), ths, mean(Ehs), tss, mean(Ess));

        i = i + 1;
      end
    end

    function draw(sweep)
      figure;

      options = struct(...
        'title', sweep.title, ...
        'xlabel', sweep.variable, ...
        'ylabel', 'log(Computational Time, s)', ...
        'marker', true);

      if sweep.includeSS
        Utils.draw(sweep.values, sweep.times, options);
        legend('HS', 'UMF', 'CE', 'SS');
      else
        Utils.draw(sweep.values, sweep.times(:, 1:3), options);
        legend('HS', 'UMF', 'CE');
      end

      set(gca, 'YScale', 'log');
    end
  end

  methods (Abstract, Access = protected)
    result = continueStep(i)
    config = setupStep(i)
    [ value, retake ] = valueStep(i, Tce, Tml, Ths, Tss)
  end

  methods (Access = private)
    function [ Tce, tce, Tml, tml, Ths, ths, Tss, tss ] = makeStep(sweep, i)
      config = sweep.setupStep(i);

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', 0, config{:});

      % Condensed Equation
      [ Tce, tce ] = sweep.optimaSolveOnAverage(param_line);

      % HotSpot
      [ Ths, ths ] = sweep.optimaVerifyOnAverage(param_line);

      % UMF in Matlab
      [ Tml, tml ] = sweep.matlabOnAverage(param_line);

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'precise_steady_state', ...
          'leakage', 0, config{:});

      % Steady-State approximation
      [ Tss, tss ] = sweep.optimaSolveOnAverage(param_line);
    end

    function [ T, time ] = optimaSolveOnAverage(sweep, param_line)
      total = 0;

      for i = 1:sweep.tryCount
        tic;
        T = Optima.solve( ...
          sweep.system, sweep.floorplan, sweep.hotspot_config, ...
          sweep.params, param_line);
        total = total + toc;
      end

      time = total / sweep.tryCount;
    end

    function [ T, time ] = optimaVerifyOnAverage(sweep, param_line)
      total = 0;

      for i = 1:sweep.tryCount
        [ dummy, dummy, dummy, dummy, T, t ] = Optima.verify( ...
          sweep.system, sweep.floorplan, sweep.hotspot_config, ...
          sweep.params, param_line, sweep.max_iterations, sweep.tolerance);
        total = total + t;
      end

      time = total / sweep.tryCount;
    end

    function [ T, time ] = matlabOnAverage(sweep, param_line)
      power = Optima.get_power( ...
        sweep.system, sweep.floorplan, sweep.hotspot_config, ...
        sweep.params, param_line);

      total = 0;

      for i = 1:sweep.tryCount
        [ T, t ] = sweep.hotspot.solve(power, 'band');
        total = total + t;
      end

      time = total / sweep.tryCount;
    end

    function e = error(sweep, T1, T2)
      e = sweep.rmse(T1, T2);
    end

    function e = rmse(sweep, T1, T2)
      e = Utils.RMSE(T1, T2);
    end

    function e = max(sweep, T1, T2)
      e = max(T2);
    end

    function e = corridor(sweep, T1, T2)
      mn1 = min(T1);
      mx1 = max(T1);
      mn2 = min(T2);
      mx2 = max(T2);
      e = (mx1 - mn1) - (mx2 - mn2);
    end
  end
end
