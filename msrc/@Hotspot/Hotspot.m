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
    function hs = Hotspot(conductance, capacitance, samplingInterval, ambientTemperature)
      hs.sinvC = diag(sqrt(1 ./ capacitance));
      hs.D = hs.sinvC * (-conductance) * hs.sinvC;
      [ V, L ] = eig(hs.D);
      hs.DL = diag(L);
      hs.DV = V;
      hs.DVT = V';
      hs.nodes = size(hs.D, 1);

      hs.samplingInterval = samplingInterval;
      hs.ambientTemperature = ambientTemperature;
    end

    function [ T, time ] = solve(hs, power)
      [ A, B ] = hs.constructSystem(power, hs.samplingInterval);

      tic;

      Y = A \ B;

      [ steps, cores ] = size(power);

      T = Utils.compact(Y, steps, cores);

      d = diag(hs.sinvC);
      d = d(1:cores);

      a = hs.ambientTemperature;

      for i = 1:steps
        T(i, :) = T(i, :) .* d + a;
      end

      time = toc;
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

    function [ A, B ] = constructSystem(hs, P, t)
      nodes = hs.nodes;
      [ steps, cores ] = size(P);

      total = nodes * steps;

      P = [ P, zeros(steps, nodes - cores) ];

      A = zeros(total, total);
      B = zeros(total, 1);

      [ expDt, G ] = hs.calculateCoefficients(t);

      for i = 1:steps
        j = (i - 1) * nodes + 1;
        k = j + nodes;
        o = nodes - 1;
        if (k > total)
          k = 1;
        end
        A(j:(j + o), j:(j + o)) = expDt;
        A(j:(j + o), k:(k + o)) = -eye(nodes);
        B(j:(j + o), 1) = - G * transpose(P(i, :));
      end
    end
  end
end
