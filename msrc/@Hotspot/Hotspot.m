classdef Hotspot < handle
  properties (SetAccess = private)
    sinvC
    D
    DL
    DV
    DVT
    nodes

    samplingInterval
    ambientTemperature
  end

  methods
    function hs = Hotspot(floorplan, config, config_line)
      [ conductance, capacitance, inversed_capacitance ] = Optima.get_coefficients( ...
        floorplan, config, config_line);

      hs.sinvC = diag(sqrt(inversed_capacitance));
      hs.D = hs.sinvC * (-conductance) * hs.sinvC;
      [ V, L ] = eig(hs.D);
      hs.DL = diag(L);
      hs.DV = V;
      hs.DVT = V';
      hs.nodes = size(hs.D, 1);

      hs.samplingInterval = Utils.readParameter(config, '-sampling_intvl');
      hs.ambientTemperature = Utils.readParameter(config, '-ambient');
    end

    function [ T, time ] = solve(hs, power)
      start = tic;

      [ A, B ] = hs.constructBand(power, hs.samplingInterval);

      Y = A \ B;

      [ steps, cores ] = size(power);

      T = Utils.compact(Y, steps, cores);

      d = diag(hs.sinvC);
      d = transpose(d(1:cores));

      a = hs.ambientTemperature;

      for i = 1:steps
        T(i, :) = T(i, :) .* d + a;
      end

      time = toc(start);
    end
  end

  methods (Access = private)
    function [ expDt, G ] = calculateCoefficients(hs, t)
      % exp(D * t) = U diag(exp(li * t)) UT
      %
      expDt = hs.DV * diag(exp(t * hs.DL)) * hs.DVT;

      % G = D^(-1) (exp(D * t) - I) C^(-1/2) =
      % U diag((exp(li * t) - 1) / li) UT C^(-1/2)
      %
      G = hs.DV * diag((exp(t * hs.DL) - 1) ./ hs.DL) * hs.DVT * hs.sinvC;
    end

    function [ A, B ] = constructFull(hs, P, t)
      [ m, cores ] = size(P);
      n = hs.nodes;

      nm = n * m;

      P = [ P, zeros(m, n - cores) ];

      A = zeros(nm, nm);
      B = zeros(nm, 1);

      [ expDt, G ] = hs.calculateCoefficients(t);

      for i = 1:m
        j = (i - 1) * n + 1;
        k = j + n;
        o = n - 1;
        if (k > nm)
          k = 1;
        end
        A(j:(j + o), j:(j + o)) = expDt;
        A(j:(j + o), k:(k + o)) = -eye(n);
        B(j:(j + o), 1) = - G * transpose(P(i, :));
      end
    end

    function [ A, B ] = constructBand(hs, P, t)
      [ m, cores ] = size(P);
      n = hs.nodes;

      nm = n * m;

      P = [ P, zeros(m, n - cores) ];

      B = zeros(nm, 1);

      m1 = n;
      m2 = n;
      p = m1 + 1 + m2;
      d = [ -(nm - n), -(n - 1):(n - 1), n ];
      A = zeros(nm, p);

      [ expDt, G ] = hs.calculateCoefficients(t);

      for s = 1:m
        o = 0;
        for j = 1:n
          r = (s - 1) * n + j;
          for i = 1:n
            A(r, m1 + 1 - (i - 1) + o) = expDt(i, j);
          end
          o = o + 1;
        end
        r = (s - 1) * n + 1;
        B(r:(r + n - 1), 1) = - G * transpose(P(s, :));
      end

      A(:, 1) = -ones(nm, 1);
      A(:, p) = -ones(nm, 1);

      A = spdiags(A, d, nm, nm);
    end
  end
end
