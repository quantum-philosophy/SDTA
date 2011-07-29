classdef TM < handle
  properties (Constant)
    samplingInterval = 0.7e-3;    % Sampling interval
    ambientTemperature = 45.0;  % Ambient temperature
  end

  properties (Access = private)
    floorplan
    config
  end

  methods
    function tm = TM(floorplan, config)
      tm.floorplan = floorplan;
      tm.config = config;
    end

    function T = solveWithCondensedEquation(tm, B)
      [ negA, invC ] = External.obtainHotSpotModel(tm.floorplan, tm.config);
      invC = diag(diag(invC));

      sinvC = sqrt(invC);
      D = Utils.symmetrize(sinvC * (- negA) * sinvC);

      [ V, L ] = eig(D);

      DL = diag(L);
      DV = V;
      DVT = V';
      sinvC = sinvC;
      nodeCount = length(D);

      n = length(D);
      m = size(B, 1);
      cores = size(B, 2);
      nm = n * m;

      ts = tm.samplingInterval;
      at = tm.ambientTemperature;

      B = transpose(B);
      B = [ B; zeros(n - cores, m) ];

      % exp(D * t) = U * diag(exp(li * t)) * UT
      %
      K = DV * diag(exp(ts * DL)) * DVT;

      % G = D^(-1) (exp(D * t) - I) C^(-1/2) =
      % U * diag((exp(li * t) - 1) / li) * UT * C^(-1/2)
      %
      G = DV * diag((exp(ts * DL) - 1) ./ DL) * DVT * sinvC;

      P = zeros(n, m);
      Q = zeros(n, m);
      Q(:, 1) = G * B(:, 1);
      P(:, 1) = Q(:, 1);
      for i = 2:m
        Q(:, i) = G * B(:, i);
        P(:, i) = K * P(:, i - 1) + Q(:, i);
      end

      Y = zeros(nm, 1);
      Y(1:n) = DV * diag(1 ./ (1 - exp(ts * m * DL))) * DVT * P(:, m);

      for i = 2:m
        op = (i - 2) * n + 1;
        on = op + n;
        Y(on:(on + n - 1)) = K * Y(op:(op + n - 1)) + Q(:, i - 1);
      end

      T = zeros(n, m);
      T(:, 1) = DV * diag(1 ./ (1 - exp(ts * m * DL))) * DVT * P(:, m);

      for i = 2:m
        T(:, i) = K * T(:, i - 1) + Q(:, i - 1);
      end

      T = transpose(T(1:cores, :));
      dsinvC = transpose(diag(sinvC(1:cores, 1:cores)));

      for i = 1:m
        T(i, :) = T(i, :) .* dsinvC + at;
      end
    end

    function [ T, it ] = solveWithHotSpot(tm, B, tol, maxit)
      if nargin < 4, maxit = 10; end
      if nargin < 3, tol = 2; end

      steps = size(B, 1);
      cores = size(B, 2);
      nodes = 4 * cores + 12;

      % ATTENTION: B is a steps-by-cores matrix right now. Because of the fact
      % than MatLab stores matrices column by column, not row by row as
      % it is in C/C++, the external code will get uncomfortable formatted
      % data. No eliminate extra transformations there, we do them here.
      % Plus provide HotSpot with zero power slots.
      B = transpose(B);
      B = [ B; zeros(nodes - cores, steps) ];
      tic
      [ T, it ] = External.solveSSDTCWithHotSpot(...
        tm.floorplan, tm.config, B, tol, maxit);
      toc
      T = transpose(T(1:cores, :)) - 273.15;

      if it == maxit
        fprintf('HotSpot exceeded the maximal number of iterations\n');
      else
        fprintf('HotSpot finished in %d iterations\n', it);
      end
    end
  end
end
