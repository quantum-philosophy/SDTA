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

    K
    G
    R

    decompositionTime
    preparationTime
  end

  methods
    function hs = Hotspot(floorplan, config, config_line)
      preparation = tic;

      [ conductance, capacitance, inversed_capacitance ] = Optima.get_coefficients( ...
        floorplan, config, config_line);

      hs.sinvC = diag(sqrt(1 ./ capacitance));
      hs.D = Utils.symmetrize(hs.sinvC * (-conductance) * hs.sinvC);

      decomposition = tic;
      [ V, L ] = eig(hs.D);
      hs.decompositionTime = toc(decomposition);

      hs.DL = diag(L);
      hs.DV = V;
      hs.DVT = V';
      hs.nodes = size(hs.D, 1);

      hs.samplingInterval = Utils.readParameter(config, '-sampling_intvl');
      hs.ambientTemperature = Utils.readParameter(config, '-ambient');

      [ hs.K, hs.G ] = hs.calculateCoefficients(hs.samplingInterval);

      hs.R = hs.DV * diag(-1 ./ hs.DL) * hs.DVT * hs.sinvC;

      hs.preparationTime = toc(preparation);
    end

    function [ T, time ] = solve(hs, power, method)
      if nargin < 3, method = 'ce'; end

      start = tic;

      Y = hs.(method)(power);

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

    function T = steady(hs, P)
      cores = length(P);
      n = hs.nodes;
      P = [ P(:); zeros(n - cores, 1) ];
      T = hs.R * P;
      T = diag(hs.sinvC) .* T + hs.ambientTemperature;
      T = T(1:cores);
    end

    function [ A, B ] = constructFull(hs, P, t)
      [ m, cores ] = size(P);
      n = hs.nodes;

      nm = n * m;

      P = [ P, zeros(m, n - cores) ];

      A = zeros(nm, nm);
      B = zeros(nm, 1);

      expDt = hs.K;
      G = hs.G;

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

      expDt = hs.K;
      G = hs.G;

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

    function [ expDt, G ] = calculateCoefficients(hs, t)
      % exp(D * t) = U diag(exp(li * t)) UT
      %
      expDt = hs.DV * diag(exp(t * hs.DL)) * hs.DVT;

      % G = D^(-1) (exp(D * t) - I) C^(-1/2) =
      % U diag((exp(li * t) - 1) / li) UT C^(-1/2)
      %
      G = hs.DV * diag((exp(t * hs.DL) - 1) ./ hs.DL) * hs.DVT * hs.sinvC;
    end
  end

  methods (Access = private)
    function Y = band(hs, P)
      [ A, B ] = hs.constructBand(P, hs.samplingInterval);
      Y = A \ B;
    end

    function Y = ce(hs, B)
      [ m, cores ] = size(B);
      n = hs.nodes;
      nm = n * m;

      B = [ B, zeros(m, n - cores) ];

      K = hs.K;
      G = hs.G;

      P = zeros(n, m);
      Q = zeros(n, m);
      Q(:, 1) = G * transpose(B(1, :));
      P(:, 1) = Q(:, 1);
      for i = 2:m
        Q(:, i) = G * transpose(B(i, :));
        P(:, i) = K * P(:, i - 1) + Q(:, i);
      end

      Y = zeros(nm, 1);
      Y(1:n) = hs.DV * diag(1 ./ (1 - exp(hs.samplingInterval * m * hs.DL))) * hs.DVT * P(:, m);

      for i = 2:m
        op = (i - 2) * n + 1;
        on = op + n;
        Y(on:(on + n - 1)) = K * Y(op:(op + n - 1)) + Q(:, i - 1);
      end
    end

    function Y = ta(hs, B)
      [ m, cores ] = size(B);
      n = hs.nodes;

      B = [ B, zeros(m, n - cores) ];

      K = hs.K;
      G = hs.G;

      Y = zeros(n * m, 1);
      Y(1:n) = G * transpose(B(1, :));

      for i = 2:m
        op = (i - 2) * n + 1;
        on = op + n;
        Y(on:(on + n - 1)) = K * Y(op:(op + n - 1)) + G * transpose(B(i - 1, :));
      end
    end

    function Y = ss(hs, B)
      [ m, cores ] = size(B);
      n = hs.nodes;

      B = [ B, zeros(m, n - cores) ];

      Y = zeros(n * m, 1);
      R = hs.R;

      for i = 1:m
        op = (i - 1) * n + 1;
        Y(op:(op + n - 1)) = R * transpose(B(i, :));
      end
    end

    function YY = bc(hs, B)
      [ m, cores ] = size(B);
      n = hs.nodes;
      nm = n * m;

      B = [ B, zeros(m, n - cores) ];

      t = hs.samplingInterval;

      [ expDt, G ] = hs.calculateCoefficients(t);

      AA = zeros(2, n, n);
      AA(1, :, :) = expDt;
      AA(2, :, :) = -eye(n);

      BB = zeros(n, m);

      for i = 1:m
        BB(:, i) = - G * transpose(B(i, :));
      end

      % Solve
      %
      AA = conj(fft(AA, m, 1));
      BB = fft(BB, m, 2);

      YY = zeros(n, m);

      for i = 1:m
        YY(:, i) = squeeze(AA(i, :, :)) \ BB(:, i);
      end

      YY = Utils.flatten(ifft(YY, m, 2));
    end
  end
end
