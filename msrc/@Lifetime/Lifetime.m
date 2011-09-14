classdef Lifetime < handle
  properties (Constant)
    % Peak threshold of local minima and maxima (for the cycle counting)
    peakThreshold = 2; % K

    % Miner's rule constant [1]
    % sum(ni / Ni) = C
    C = 1; % from 0.7 to 2.2, usually just 1

    % Coffin-Manson equation [2]
    % Nf = C0 * (dT - dT0)^(-q)

    % Coffin-Manson exponent [2]
    q = 6; % from 6 to 9 for brittle fracture (Si and dielectrics: SiO2, Si3N4)

    % Portion of the temperature range in the elastic region [2]
    dT0 = 0;

    % Coffin-Manson equation with the Arrhenius term [3]
    % Ntc = Atc * (dT - dT0)^(-q) * exp(Eatc / (k * Tmax))

    % Activation energy [4], [5]
    Eatc = 0.7; % eV, depends on particular failure mechanism and
    % material involved, typically ranges from 0.3 up to 1.5

    % Boltzmann constant [6]
    k = 8.61733248e-5; % eV/K

    % Empirically determined constant
    Atc = 1; % Lifetime.calculateAtc;

    % Shape parameter for the Weibull distribution
    beta = 2;
  end

  methods (Static)
    function mttf = predict(T)
      mttf = Lifetime.predictCombined(T);
    end

    function [ mttf, maxp, minp, cycles ] = predictSingle(T)
      time = size(T, 1) * Constants.samplingInterval;

      [ damage, maxp, minp, cycles ] = Lifetime.calculateDamage(T);

      mttf = time * Lifetime.C / damage;
    end

    function mttf = predictCombined(T)
      time = size(T, 1) * Constants.samplingInterval;

      factor = 0;
      for i = 1:size(T, 2)
        damage = Lifetime.calculateDamage(T(:, i));
        factor = factor + damage ^ Lifetime.beta;
      end

      damage = factor ^ (1 / Lifetime.beta);
      mttf = time / damage;
    end

    function mttf = predictCombinedAndDraw(T)
      time = size(T, 1) * Constants.samplingInterval;

      G = gamma(1 + 1 / Lifetime.beta);

      figure;

      colors = Constants.roundRobinColors;
      title = {};

      factor = 0;
      for i = 1:size(T, 2)
        damage = Lifetime.calculateDamage(T(:, i));
        factor = factor + damage ^ Lifetime.beta;

        mttf = time / damage;
        eta = time / (G * damage);

        color = colors{mod(i - 1, length(colors)) + 1};
        s = mttf / 100;
        t = ((1:100) - 1) * s;
        r = exp(-(t ./ eta).^Lifetime.beta);
        line(t, r, 'Color', color);
        title{end + 1} = [ 'Core ', num2str(i) ];
      end

      damage = factor ^ (1 / Lifetime.beta);

      mttf = time / damage;
      eta = time / (G * damage);

      s = mttf / 100;
      t = ((1:100) - 1) * s;
      r = exp(-(t ./ eta).^Lifetime.beta);
      line(t, r, 'Color', 'k', 'Line', '--');
      title{end + 1} = 'Total';

      legend(title{:});
    end

    function [ mttf, cycles ] = predictMultiple(T)
      mttf = zeros(0);
      cycles = zeros(0);
      for i = 1:size(T, 2)
        [ mttf(end + 1), dummy, dummy, discreteCycle ] = ...
          Lifetime.predictSingle(T(:, i));
        cycles(end + 1) = sum(discreteCycle);
      end
    end

    function [ mttf, cycles ] = predictMultipleAndDraw(T)
      mttf = zeros(0);
      cycles = zeros(0);

      figure;

      [ steps, cores ] = size(T);

      index = zeros(steps, cores);

      cycleLegend = {};

      for i = 1:size(T, 2)
        [ mttf(end + 1), maxp, minp, discreteCycle ] = ...
          Lifetime.predictSingle(T(:, i));
        cycles(end + 1) = sum(discreteCycle);

        cycleLegend{end + 1} = sprintf('%.2f cycles', cycles(end));

        I = sort([ maxp(:, 1); minp(:, 1) ]);
        index(1:length(I), i) = I;
      end

      x = ((1:steps) - 1) * Constants.samplingInterval;
      T = T - Constants.degreeKelvin;

      % Draw full curves
      subplot(2, 1, 1);
      Utils.drawLines('SSDTC', 'Time, s', 'Temperature, C', x, T);

      set(gca, 'XLim', [ 0 x(end) ]);
      YLim = get(gca, 'YLim');

      % Outline minima and maxima
      Utils.drawLines([], [], [], x, T, index, 'LineStyle', 'none', 'Marker', 'x');

      % Draw curves only by minima and maxima
      subplot(2, 1, 2);
      Utils.drawLines('SSDTC (only peaks)', 'Time, s', 'Temperature, C', x, T, index);

      legend(cycleLegend{:});

      set(gca, 'XLim', [ 0 x(end) ]);
      set(gca, 'YLim', YLim);
    end

    function drawCycles(T)
      mttf = zeros(0);
      cycles = zeros(0);

      [ steps, cores ] = size(T);

      index = zeros(steps, cores);

      cycleLegend = {};

      for i = 1:size(T, 2)
        [ mttf(end + 1), maxp, minp, discreteCycle ] = ...
          Lifetime.predictSingle(T(:, i));
        cycles(end + 1) = sum(discreteCycle);

        cycleLegend{end + 1} = sprintf('%d cycles', ceil(cycles(end)));

        I = sort([ maxp(:, 1); minp(:, 1) ]);
        index(1:length(I), i) = I;
      end

      x = ((1:steps) - 1) * Constants.samplingInterval;
      T = T - Constants.degreeKelvin;

      maxT = max(max(T));
      minT = min(min(T));

      Utils.drawLines('Cycles', 'Time, s', 'Temperature, C', ...
        x, T, index);

      line([ x(1), x(end) ], [ minT, minT ], 'Line', '--', 'Color', 'k');
      cycleLegend{end + 1} = [ 'Tmin (', num2str(Utils.round2(minT, 0.01)), ' C)' ];

      line([ x(1), x(end) ], [ maxT, maxT ], 'Line', '-.', 'Color', 'k');
      cycleLegend{end + 1} = [ 'Tmax (', num2str(Utils.round2(maxT, 0.01)), ' C)' ];

      legend(cycleLegend{:});
    end

    function Atc = calculateAtc
      % Let us assume to have the mean time to failure equal to 20 years
      % with the average temperature of 60 C, the total application period
      % of 200ms, and 100 equal cycles of 10 C.

      mttf = 100 * 365 * 24 * 60 * 30; % s, 20 years
      Tavg = 60 + Constants.degreeKelvin; % K, 60 C
      totalTime = 1; % s
      m = 50; % Number of cycles
      dT = 10; % K
      Tmax = Tavg + dT / 2;

      factor = m * (dT - Lifetime.dT0)^Lifetime.q * ...
        exp(-Lifetime.Eatc / (Lifetime.k * Tmax));

      Atc = mttf * factor / totalTime;
    end
  end

  methods (Static, Access = private)
    function [ damage, maxp, minp, cycles ] = calculateDamage(T)
      % Get extremum
      [ maxp, minp ] = Utils.peakdet(T, Lifetime.peakThreshold);

      % Combine maxima and minima indexes
      I = sort([ maxp(:, 1); minp(:, 1) ]);

      % Get the actual temperature values
      T = T(I);

      % Rainflow it!
      rainflow = Rainflow.rainflow(T);

      % Amplitudes
      dT = rainflow(1, :);

      % Maximal temperatures during each cycle
      Tmax = rainflow(2, :) + dT ./ 2;

      % Contains full cycles (1.0) and half cycles (0.5)
      cycles = rainflow(3, :);

      % Number of cycles to failure for each stress level [3]
      N = Lifetime.Atc .* (dT - Lifetime.dT0).^(-Lifetime.q) .* ...
        exp(Lifetime.Eatc ./ (Lifetime.k * Tmax));

      % Count all detected cycles (even 0.5) as completed,
      % since we have cycling temperature fluctuations
      % (1 instead of cycles here)
      damage = sum(1 ./ N);
    end
  end
end

% References:
% [1] http://en.wikipedia.org/wiki/Fatigue_(material)
% [2] Failure Mechanisms and Models for Semiconductor Devices
% [3] System-Level Reliability Modeling for MPSoCs
% [4] http://www.siliconfareast.com/activation-energy.htm
% [5] http://rel.intersil.com/docs/rel/calculation_of_semiconductor_failure_rates.pdf
% [6] http://en.wikipedia.org/wiki/Boltzmann_constant
