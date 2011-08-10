classdef HotSpot < handle
  properties (Access = private)
    floorplan
    config
  end

  methods
    function hs = HotSpot(floorplan, config)
      hs.floorplan = floorplan;
      hs.config = config;
    end

    function T = solveNativeCondensedEquation(hs, B)
      [ D, sinvC ] = hs.obtainCoefficients();

      if size(D, 1) ~= 4 * size(B, 2) + 12
        error('HotSpot:solveCondensedEquation', ...
          'The floorplan does not match the task case');
      end

      [ V, L ] = eig(D);

      L = diag(L);
      VT = V';

      n = length(D);
      m = size(B, 1);
      cores = size(B, 2);
      nm = n * m;

      ts = Constants.samplingInterval;
      at = Constants.ambientTemperature;

      B = transpose(B);
      B = [ B; zeros(n - cores, m) ];

      [ K, G ] = hs.calculateConstants(L, V, VT, sinvC, ts);

      P = zeros(n, m);
      Q = zeros(n, m);
      Q(:, 1) = G * B(:, 1);
      P(:, 1) = Q(:, 1);
      for i = 2:m
        Q(:, i) = G * B(:, i);
        P(:, i) = K * P(:, i - 1) + Q(:, i);
      end

      T = zeros(n, m);
      T(:, 1) = V * diag(1 ./ (1 - exp(ts * m * L))) * VT * P(:, m);

      for i = 2:m
        T(:, i) = K * T(:, i - 1) + Q(:, i - 1);
      end

      T = transpose(T(1:cores, :));
      dsinvC = transpose(diag(sinvC(1:cores, 1:cores)));

      for i = 1:m
        T(i, :) = T(i, :) .* dsinvC + at;
      end
    end

    function T = solveCondensedEquation(hs, B)
      options = struct(...
        'ambient', Constants.ambientTemperature, ...
        'sampling_intvl', Constants.samplingInterval);

      % ATTENTION: B is a steps-by-cores matrix right now. Because of the fact
      % than MatLab stores matrices column by column, not row by row as
      % it is in C/C++, the external code will get uncomfortable formatted
      % data. To eliminate extra transformations there, we do them here.
      B = transpose(B);
      % External call
      T = hs.solve_condensed_equation(B, options);
      T = transpose(T);
    end

    function [ T, it ] = solveOriginal(hs, B, tol, minbad, maxit)
      if nargin < 3, tol = 2; end
      if nargin < 4, minbad = 0; end
      if nargin < 5, maxit = 10; end

      steps = size(B, 1);
      cores = size(B, 2);
      nodes = 4 * cores + 12;

      % ATTENTION: B is a steps-by-cores matrix right now. Because of the fact
      % than MatLab stores matrices column by column, not row by row as
      % it is in C/C++, the external code will get uncomfortable formatted
      % data. To eliminate extra transformations there, we do them here.
      % Plus provide HotSpot with zero power slots.
      B = transpose(B);
      B = [ B; zeros(nodes - cores, steps) ];
      % External call
      [ T, it ] = hs.solve_original(B, tol, minbad, maxit);
      T = transpose(T(1:cores, :));

      if it == maxit
        fprintf('HotSpot exceeded the maximal number of iterations\n');
      else
        fprintf('HotSpot finished in %d iterations\n', it);
      end
    end

    function T = solvePlainOriginal(hs, power, steps, repeat)
      powerEx = [ power, sprintf('_x_%d', repeat) ];

      if isempty(strfind(powerEx, 'ptrace'))
        tempEx = [ powerEx, '.ttrace' ];
      else
        tempEx = strrep(powerEx, 'ptrace', 'ttrace');
      end

      Utils.startTimer('Extend the power profile');
      Utils.extendPowerProfile(power, powerEx, repeat);
      Utils.stopTimer();

      status = Utils.run(sprintf('hotspot-%s', Constants.hotspotVersion), ...
        '-f', hs.floorplan, ...
        '-p', powerEx, ...
        '-c', hs.config, ...
        '-o', tempEx);

      if status ~= 0
        error('HotSpot:solvePlainHotSpot', 'Cannot execute HotSpot');
      end

      % Skip the header line and all excessive repetitions
      T = dlmread(tempEx, '\t', 1 + (repeat - 1) * steps, 0);
    end

    function T = solveBlockCirculant(hs, B)
      [ D, sinvC ] = hs.obtainCoefficients();

      if size(D, 1) ~= 4 * size(B, 2) + 12
        error('HotSpot:solveBlockCirculant', ...
          'The floorplan does not match the task case');
      end

      [ V, L ] = eig(D);

      L = diag(L);
      VT = V';

      n = length(D);
      m = size(B, 1);
      cores = size(B, 2);
      nm = n * m;

      ts = Constants.samplingInterval;
      at = Constants.ambientTemperature;

      B = transpose(B);
      B = [ B; zeros(n - cores, m) ];

      [ K, G ] = hs.calculateConstants(L, V, VT, sinvC, ts);

      AA = zeros(2, n, n);
      AA(1, :, :) = K;
      AA(2, :, :) = -eye(n);

      BB = zeros(n, m);

      for i = 1:m
        BB(:, i) = - G * B(:, i);
      end

      AA = conj(fft(AA, m, 1));
      BB = fft(BB, m, 2);

      YY = zeros(n, m);

      for i = 1:m
        YY(:, i) = squeeze(AA(i, :, :)) \ BB(:, i);
      end

      YY = transpose(ifft(YY, m, 2));

      dsinvC = transpose(diag(sinvC(1:cores, 1:cores)));

      T = zeros(m, cores);

      for i = 1:m
        T(i, :) = YY(i, 1:cores) .* dsinvC + at;
      end
    end
  end

  methods (Access = private)
    function [ D, sinvC ] = obtainCoefficients(hs)
      % External call
      [ negA, dinvC ] = hs.obtain_coefficients();

      sinvC = sqrt(diag(dinvC));
      D = Utils.symmetrize(sinvC * (- negA) * sinvC);
    end

    function [ K, G ] = calculateConstants(hs, L, V, VT, sinvC, ts)
      % exp(D * t) = U * diag(exp(li * t)) * UT
      K = V * diag(exp(ts * L)) * VT;

      % G = D^(-1) (exp(D * t) - I) C^(-1/2) =
      % U * diag((exp(li * t) - 1) / li) * UT * C^(-1/2)
      G = V * diag((exp(ts * L) - 1) ./ L) * VT * sinvC;
    end
  end
end
