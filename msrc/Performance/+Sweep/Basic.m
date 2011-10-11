classdef Basic < handle
  properties (Constant)
    max_iterations = 2;
    tolerance = 0;
    hotspot_line = '';
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

      sweep.title = 'Computational time';
      sweep.variable = 'Variable';

      sweep.hotspot = Hotspot(sweep.floorplan, ...
        sweep.hotspot_config, sweep.hotspot_line);
    end

    function perform(sweep)
      fprintf('%15s%15s%15s%15s%15s\n', ...
        'CE, s', 'UMF, s', 'UMF RMSE', 'HotSpot, s', 'HotSpot RMSE');

      sweep.values = zeros(0, 0);
      sweep.times = zeros(0, 3);

      i = 1;

      while sweep.continueStep(i)
        while true
          [ Tce, tce, Tml, tml, Ths, ths ] = sweep.makeStep(i);
          [ value, retake ] = sweep.valueStep(i, Tce, Tml, Ths);
          if ~retake, break; end
        end

        sweep.values(end + 1) = value;

        sweep.times(end + 1, 1:3) = [ tce, tml, ths ];

        Eml = Utils.RMSE(Tce, Tml);
        Ehs = Utils.RMSE(Tce, Ths);

        fprintf('%15.4f%15.4f%15.2e%15.4f%15.2e\n', ...
          tce, tml, mean(Eml), ths, mean(Ehs));

        i = i + 1;
      end
    end

    function draw(sweep)
      figure;

      options = struct(...
        'title', sweep.title, ...
        'xlabel', sweep.variable, ...
        'ylabel', 'log(Time, s)');

      Utils.draw(sweep.values, sweep.times, options, 'Marker', 'o');
      set(gca, 'YScale', 'log');

      legend('CE', 'UMF', 'HotSpot');
    end
  end

  methods (Abstract, Access = protected)
    result = continueStep(i)
    config = setupStep(i)
    [ value, retake ] = valueStep(i, Tce, Tss, power)
  end

  methods (Access = private)
    function [ Tce, tce, Tml, tml, Ths, ths ] = makeStep(sweep, i)
      config = sweep.setupStep(i);

      param_line = Utils.configStream(...
        'verbose', 0, ...
        'steady_state', 0, ...
        'leakage', 0, config{:});

      tce = 0;

      for i = 1:10
        start = tic;
        [ Tce, power, dummy ] = Optima.solve( ...
          sweep.system, sweep.floorplan, sweep.hotspot_config, ...
          sweep.params, param_line);
        tce = tce + toc(start);
      end

      tce = tce / 10;

      % Condensed Equation and HotSpot
      [ dummy, dummy, dummy, dummy, Ths, ths ] = Optima.verify( ...
        sweep.system, sweep.floorplan, sweep.hotspot_config, ...
        sweep.params, param_line, sweep.max_iterations, sweep.tolerance);

      % UMF in Matlab
      [ Tml, tml ] = sweep.hotspot.solve(power, 'band');
    end
  end
end
