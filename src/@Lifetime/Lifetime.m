classdef Lifetime < handle
  properties (Constant)
    % Peak threshold of local minima and maxima (for the cycle counting)
    peakThreshold = 1.0; % K

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

    % Empirically determined constant
    Atc = 1;

    % Activation energy [4], [5]
    Eatc = 0.7; % eV, depends on particular failure mechanism and
    % material involved, typically ranges from 0.3 up to 1.5

    % Boltzmann constant [6]
    k = 8.61733248e-5; % eV/K

    % Shortcut for Eatc / k
    beta =  Lifetime.Eatc / Lifetime.k;
  end

  methods (Static)
    function [ mttf, maxp, minp, cycles ] = predictSingle(T, totalTime)
      if nargin < 2, totalTime = size(T, 1) * Constants.samplingInterval; end

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
        exp(Lifetime.beta ./ Tmax);

      totalDamage = sum(cycles ./ N);
      mttf = totalTime * Lifetime.C / totalDamage;
    end

    function mttf = predict(T, varargin)
      mttf = zeros(0, 0);
      for i = 1:size(T, 2)
        mttf(end + 1) = Lifetime.predictSingle(T(:, i), varargin{:});
      end
    end

    function mttf = predictAndDraw(T, varargin)
      mttf = zeros(0, 0);

      figure;

      [ steps, cores ] = size(T);

      index = zeros(steps, cores);

      for i = 1:size(T, 2)
        [ mttf(end + 1), maxp, minp, cycles ] = ...
          Lifetime.predictSingle(T(:, i), varargin{:});

        I = sort([ maxp(:, 1); minp(:, 1) ]);
        index(1:length(I), i) = I;
      end

      x = ((1:steps) - 1) * Constants.samplingInterval;
      T = T - Constants.degreeKelvin;

      % Draw full curves
      subplot(2, 1, 1);
      Utils.drawLines('SSDTC', 'Time, s', 'Temperature, C', x, T);

      % Outline minima and maxima
      Utils.drawLines([], [], [], x, T, index, 'LineStyle', 'none', 'Marker', 'x');

      % Draw curves only by minima and maxima
      subplot(2, 1, 2);
      Utils.drawLines('SSDTC (only peaks)', 'Time, s', 'Temperature, C', x, T, index);
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
