classdef Basic < handle
  properties (Constant)
    maxIterations = 1;
    tolerance = 0;

    hotspot_line = '';

    tryCount = 30;
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
    legend

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
      sweep.legend = { ...
        'Condensed Equation Method', ...
        'One HotSpot Simulation', ...
        'Steady-State Approximation', ...
        'Unsymmetric MultiFrontal Method', ...
        'FFT Method (Block-Circulant)' };

      sweep.hotspot = Hotspot(sweep.floorplan, ...
        sweep.hotspot_config, sweep.hotspot_line);
    end

    function perform(sweep)
      fprintf('%15s%15s%15s%15s%15s%15s%15s%15s%15s\n', ...
        'CE, s', ...
        'HS, s', 'Error(HS)', ...
        'SS, s', 'Error(SS)', ...
        'UMF, s', 'Error(UMF)', ...
        'BC, s', 'Error(BC)');

      sweep.values = zeros(0, 0);
      sweep.times = zeros(0, 5);

      i = 1;

      while sweep.continueStep(i)
        while true
          [ T, t ] = sweep.makeStep(i);
          [ value, retake ] = sweep.valueStep(i, T, t);
          if ~retake, break; end
        end

        sweep.values(end + 1) = value;
        sweep.times(end + 1, 1:length(t)) = t;

        fprintf('%15.4f', t(1));

        for j = 2:size(T, 1)
          E = sweep.error(T(1, :, :), T(j, :, :));
          fprintf('%15.4f%15.2e', t(j), mean(E));
        end

        fprintf('\n');

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
      legend(sweep.legend{:});

      set(gca, 'YScale', 'log');
    end
  end

  methods (Abstract, Access = protected)
    result = continueStep(i)
    config = setupStep(i)
    [ value, retake ] = valueStep(i, T, t)
  end

  methods (Access = private)
    function [ T, t ] = makeStep(sweep, i)
      config = sweep.setupStep(i);

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', 0, config{:});

      T = zeros(5, 0, 0);
      t = zeros(1, 5);

      % Condensed Equation
      [ temp, t(1) ] = sweep.optimaSolveOnAverage(param_line);
      T(1, 1:size(temp, 1), 1:size(temp, 2)) = temp;

      % HotSpot
      [ temp, t(2) ] = sweep.optimaVerifyOnAverage(param_line);
      T(2, 1:size(temp, 1), 1:size(temp, 2)) = temp;

      % UMF in Matlab
      [ temp, t(4) ] = sweep.matlabOnAverage(param_line, 'band');
      T(4, 1:size(temp, 1), 1:size(temp, 2)) = temp;

      % Block-Circulant
      [ temp, t(5) ] = sweep.matlabOnAverage(param_line, 'bc');
      T(5, 1:size(temp, 1), 1:size(temp, 2)) = temp;

      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'precise_steady_state', ...
          'leakage', 0, config{:});

      % Steady-State approximation
      [ temp, t(3) ] = sweep.optimaSolveOnAverage(param_line);
      T(3, 1:size(temp, 1), 1:size(temp, 2)) = temp;
    end

    function [ T, time ] = optimaSolveOnAverage(sweep, param_line)
      total = 0;

      for i = 1:sweep.tryCount
        [ T, dummy, t ] = Optima.solve( ...
          sweep.system, sweep.floorplan, sweep.hotspot_config, ...
          sweep.params, param_line);
        total = total + t;
      end

      time = total / sweep.tryCount;
    end

    function [ T, time ] = optimaVerifyOnAverage(sweep, param_line)
      total = 0;

      for i = 1:sweep.tryCount
        [ dummy, dummy, dummy, dummy, T, t ] = Optima.verify( ...
          sweep.system, sweep.floorplan, sweep.hotspot_config, ...
          sweep.params, param_line, sweep.maxIterations, sweep.tolerance);
        total = total + t;
      end

      time = total / sweep.tryCount;
    end

    function [ T, time ] = matlabOnAverage(sweep, param_line, method)
      if nargin < 3, method = 'band'; end

      power = Optima.get_power( ...
        sweep.system, sweep.floorplan, sweep.hotspot_config, ...
        sweep.params, param_line);

      total = 0;

      for i = 1:sweep.tryCount
        [ T, t ] = sweep.hotspot.solve(power, method);
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
