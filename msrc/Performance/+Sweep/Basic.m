classdef Basic < handle
  properties (Constant)
    max_iterations = 2;
    tolerance = 0;
    hotspot_line = '';

    tryCount = 10;
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

        sweep.times(end + 1, 1:4) = [ tce, tml, ths, tss ];

        Eml = sweep.error(Tce, Tml);
        Ehs = sweep.error(Tce, Ths);
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

      Utils.draw(sweep.values, sweep.times, options);
      set(gca, 'YScale', 'log');

      legend('CE', 'UMF', 'HS', 'SS');
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
      [ Tce, tce ] = sweep.solveOnAverage( ...
        sweep.system, sweep.floorplan, sweep.hotspot_config, ...
        sweep.params, param_line);

      % HotSpot
      [ dummy, dummy, dummy, dummy, Ths, ths ] = Optima.verify( ...
        sweep.system, sweep.floorplan, sweep.hotspot_config, ...
        sweep.params, param_line, sweep.max_iterations, sweep.tolerance);

      % UMF in Matlab
      power = Optima.get_power( ...
        sweep.system, sweep.floorplan, sweep.hotspot_config, ...
        sweep.params, param_line);

      [ Tml, tml ] = sweep.hotspot.solve(power, 'band');

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'precise_steady_state', ...
          'leakage', 0, config{:});

      % Steady-State approximation
      [ Tss, tss ] = sweep.solveOnAverage( ...
        sweep.system, sweep.floorplan, sweep.hotspot_config, ...
        sweep.params, param_line);
    end

    function [ out, t ] = solveOnAverage(sweep, varargin)
      t = 0;

      for i = 1:sweep.tryCount
        tic;
        out = Optima.solve(varargin{:});
        t = t + toc;
      end

      t = t / sweep.tryCount;
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
