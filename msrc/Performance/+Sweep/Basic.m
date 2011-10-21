classdef Basic < handle
  properties (Constant)
    maxIterations = 1;
    tolerance = 0;

    hotspot_line = '';

    tryCount = 10;
  end

  properties (SetAccess = protected)
    solutions
    solutionCount
    config

    title
    variable

    legend
    header

    hotspot

    values
    times
  end

  methods
    function sweep = Basic(test, solutions)
      if nargin < 2 || isempty(solutions)
        solutions = { 'ce', 'hs', 'ss', 'umf', 'fft' };
      end

      sweep.solutions = solutions;
      sweep.solutionCount = length(solutions);

      config = Optima(test);
      sweep.config = config;

      sweep.title = 'Performance';
      sweep.variable = 'Variable';

      sweep.legend = {};
      sweep.header = {};

      for i = 1:length(solutions)
        name = sweep.([ solutions{i}, 'Name' ]);
        sweep.legend{end + 1} = name.long;
        sweep.header{end + 1} = name.short;
      end

      sweep.hotspot = Hotspot(config.floorplan, ...
        config.hotspot, sweep.hotspot_line);
    end

    function perform(sweep)
      fprintf('%15s%15s', 'Parameter', 'CE, ms');

      for i = 2:sweep.solutionCount
        name = sweep.header{i};
        fprintf('%15s%15s%15s', ...
          [ name, ', ms' ], [ name, ', x' ], [ 'Error(', name, ')' ]);
      end

      fprintf('\n');

      sweep.values = zeros(0, 0);
      sweep.times = zeros(0, sweep.solutionCount);

      i = 1;

      while sweep.continueStep(i)
        while true
          [ T, t ] = sweep.makeStep(i);
          [ value, retake ] = sweep.valueStep(i, T, t);
          if ~retake, break; end
        end

        fprintf('%15s', num2str(value));

        sweep.values(end + 1) = value;
        sweep.times(end + 1, 1:length(t)) = t;

        fprintf('%15.2f', t(1) * 1e3);

        for j = 2:size(T, 1)
          E = sweep.error(T(1, :, :), T(j, :, :));
          fprintf('%15.2f%15.2f%15.2e', t(j) * 1e3, t(j) / t(1), mean(E));
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

  methods (Access = protected)
    function name = ceName(sweep)
      name = struct('short', 'CE', 'long', 'Condensed Equation Method');
    end

    function name = hsName(sweep)
      name = struct('short', 'HS', 'long', 'One HotSpot Simulation');
    end

    function name = ssName(sweep)
      name = struct('short', 'SS', 'long', 'Steady-State Approximation');
    end

    function name = umfName(sweep)
      name = struct('short', 'UMF', 'long', 'Unsymmetric MultiFrontal Method');
    end

    function name = fftName(sweep)
      name = struct('short', 'FFT', 'long', 'FFT Method (Block-Circulant)');
    end

    function [ T, t ] = ceSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', 0, config{:});

      [ T, t ] = sweep.optimaSolveOnAverage(param_line);
    end

    function [ T, t ] = hsSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', 0, config{:});

      [ T, t ] = sweep.optimaVerifyOnAverage(param_line);
    end

    function [ T, t ] = ssSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'precise_steady_state', ...
          'leakage', 0, config{:});

      [ T, t ] = sweep.optimaSolveOnAverage(param_line);
    end

    function [ T, t ] = umfSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', 0, config{:});

      [ T, t ] = sweep.matlabOnAverage(param_line, 'band');
    end

    function [ T, t ] = fftSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', 0, config{:});

      [ T, t ] = sweep.matlabOnAverage(param_line, 'bc');
    end

    function [ T, time ] = makeStep(sweep, i)
      config = sweep.setupStep(i);

      T = zeros(sweep.solutionCount, 0, 0);
      time = zeros(1, sweep.solutionCount);

      for j = 1:sweep.solutionCount
        name = sweep.solutions{j};
        [ temp, time(j) ] = sweep.([ name, 'Solve' ])(i, config);
        T(j, 1:size(temp, 1), 1:size(temp, 2)) = temp;
      end
    end

    function [ T, time ] = optimaSolveOnAverage(sweep, param_line)
      config = sweep.config;
      total = 0;

      for i = 1:sweep.tryCount
        [ T, dummy, t ] = Optima.solve( ...
          config.system, config.floorplan, config.hotspot, ...
          config.params, param_line);
        total = total + t;
      end

      time = total / sweep.tryCount;
    end

    function [ T, time ] = optimaVerifyOnAverage(sweep, param_line)
      config = sweep.config;
      total = 0;

      n = 1;

      for i = 1:n
        [ dummy, dummy, dummy, dummy, T, t ] = Optima.verify( ...
          config.system, config.floorplan, config.hotspot, ...
          config.params, param_line, sweep.maxIterations, sweep.tolerance);
        total = total + t;
      end

      time = total / n;
    end

    function [ T, time ] = matlabOnAverage(sweep, param_line, method)
      config = sweep.config;
      if nargin < 3, method = 'band'; end

      power = Optima.get_power( ...
        config.system, config.floorplan, config.hotspot, ...
        config.params, param_line);

      if strcmp(method, 'band')
        n = 1;
      else
        n = sweep.tryCount;
      end

      total = 0;

      for i = 1:n
        [ T, t ] = sweep.hotspot.solve(power, method);
        total = total + t;
      end

      time = total / n;
    end

    function e = error(sweep, T1, T2)
      e = Utils.RMSE(T1, T2);
    end
  end
end
