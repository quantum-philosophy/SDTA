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
    errors
  end

  methods
    function sweep = Basic(test, solutions)
      if nargin < 2 || isempty(solutions)
        solutions = { 'ce', 'hs', 'ss', 'umf', 'fft', 'ta' };
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
      fprintf('%15s', 'Parameter');

      for i = 1:sweep.solutionCount
        name = sweep.header{i};

        if i == 1
          fprintf('%15s',[ name, ', ms' ]);
        else
          fprintf('%15s%15s%15s', ...
            [ name, ', ms' ], [ name, ', x' ], [ 'Error(', name, ')' ]);
        end
      end

      fprintf('\n');

      sweep.values = zeros(0, 0);
      sweep.times = zeros(0, sweep.solutionCount);

      i = 1;

      while sweep.continueStep(i)
        [ T, t, value ] = sweep.makeStep(i);

        solutionCount = size(T, 1);

        fprintf('%15s', num2str(value));

        sweep.values(end + 1) = value;
        sweep.times(end + 1, 1:solutionCount) = t(:, 1);

        fprintf('%15.2f', t(1, 1) * 1e3);

        for j = 2:solutionCount
          sweep.errors(end + 1) = sweep.error(T(1, :, :), T(j, :, :));
          fprintf('%15.2f%15.2f%15.2e', t(j, 1) * 1e3, t(j, 1) / t(1, 1), sweep.errors(end));
        end

        fprintf('\n');

        for k = 2:size(t, 2)
          fprintf('%15s', '');

          fprintf('%15.2f', t(1, k) * 1e3);

          for j = 2:solutionCount
            fprintf('%15.2f%15s%15s', t(j, k) * 1e3, '', '');
          end

          fprintf('\n');
        end

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
    [ value, config ] = setupStep(i)
  end

  methods (Access = protected)
    function name = ceName(sweep)
      name = struct('short', 'CE', 'long', 'Condensed Equation Method');
    end

    function name = cemName(sweep)
      name = struct('short', 'CEm', 'long', 'Condensed Equation in Matlab');
    end

    function name = hsName(sweep)
      name = struct('short', 'HS', 'long', 'One HotSpot Simulation');
    end

    function name = ssName(sweep)
      name = struct('short', 'SS', 'long', 'Steady-State Approximation');
    end

    function name = ssmName(sweep)
      name = struct('short', 'SSm', 'long', 'Steady-State Approximation in Matlab');
    end

    function name = umfName(sweep)
      name = struct('short', 'UMF', 'long', 'Unsymmetric MultiFrontal Method');
    end

    function name = fftName(sweep)
      name = struct('short', 'FFT', 'long', 'FFT Method (Block-Circulant)');
    end

    function name = taName(sweep)
      name = struct('short', 'TA', 'long', 'One Analytical Simulation');
    end

    function name = tamName(sweep)
      name = struct('short', 'TAm', 'long', 'One Analytical Simulation in Matlab');
    end

    function [ T, t ] = ceSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', '', config{:});

      [ T, t ] = sweep.optimaSolveOnAverage(param_line);
    end

    function [ T, t ] = cemSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', '', config{:});

      [ T, t ] = sweep.matlabOnAverage(param_line, 'ce');
    end

    function [ T, t ] = hsSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'hotspot', ...
          'max_iterations', 1, ...
          'tolerance', 0, ...
          'leakage', '', config{:});

      [ T, t ] = sweep.optimaSolveOnAverage(param_line);
    end

    function [ T, t ] = ssSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'precise_steady_state', ...
          'leakage', '', config{:});

      [ T, t ] = sweep.optimaSolveOnAverage(param_line);
    end

    function [ T, t ] = ssmSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', '', config{:});

      [ T, t ] = sweep.matlabOnAverage(param_line, 'ss');
    end

    function [ T, t ] = umfSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', '', config{:});

      [ T, t ] = sweep.matlabOnAverage(param_line, 'band');
    end

    function [ T, t ] = fftSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', '', config{:});

      [ T, t ] = sweep.matlabOnAverage(param_line, 'bc');
    end

    function [ T, t ] = taSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'transient_analytical', ...
          'leakage', '', config{:});

      [ T, t ] = sweep.optimaSolveOnAverage(param_line);
    end

    function [ T, t ] = tamSolve(sweep, i, config)
      param_line = Utils.configStream(...
          'verbose', 0, ...
          'solution', 'condensed_equation', ...
          'leakage', '', config{:});

      [ T, t ] = sweep.matlabOnAverage(param_line, 'ta');
    end

    function [ T, t, value ] = makeStep(sweep, i)
      [ value, config ] = sweep.setupStep(i);

      T = zeros(sweep.solutionCount, 0, 0);
      t = zeros(sweep.solutionCount, 0);

      for j = 1:sweep.solutionCount
        name = sweep.solutions{j};
        [ temp, time ] = sweep.([ name, 'Solve' ])(i, config);
        t(j, 1:length(time)) = time;
        T(j, 1:size(temp, 1), 1:size(temp, 2)) = temp;
      end
    end

    function [ T, time ] = optimaSolveOnAverage(sweep, param_line)
      config = sweep.config;

      for i = 1:sweep.tryCount
        [ T, dummy, t ] = Optima.solve( ...
          config.system, config.floorplan, config.hotspot, ...
          config.params, param_line);

        if i == 1
          total = t;
        else
          total = total + t;
        end
      end

      time = total / sweep.tryCount;
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

      time = [ time, 0, sweep.hotspot.decompositionTime, sweep.hotspot.preparationTime ];
    end

    function e = error(sweep, T1, T2)
      e = Utils.NRMSE(T1, T2) * 100;
    end
  end
end
