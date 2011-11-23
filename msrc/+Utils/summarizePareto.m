function summarizePareto(location, specific)
  if nargin < 2, specific = []; end

  colors = Constants.roundRobinColors;

  files = dir([ location, '/*.log' ]);
  count = length(files);

  fprintf('%5s%15s%15s%15s%15s%15s%15s\n', '#', 'min MTTF', 'max MTTF', ...
    'min E', 'max E', 'delta MTTF, %', 'delta E, %');

  Lifetime = [];
  Energy = [];

  N = 100;

  deltaL = [];
  deltaE = [];

  S = zeros(0, N);

  for i = 1:count
    file = sprintf('%s/%s', location, files(i).name);
    fid = fopen(file);

    L0 = 0;
    E0 = 0;

    str = fgetl(fid);
    while ischar(str)
      tokens = regexp(str, '^\s*(\w[\w\s]+):\s*(.*)$', 'tokens');

      if isempty(tokens)
        str = fgetl(fid);
        continue;
      end

      name = tokens{1}{1};
      value = tokens{1}{2};

      switch (name)
      case 'Initial lifetime'
        L0 = str2num(value);
      case 'Initial energy'
        E0 = str2num(value);
      case 'Pareto optima'
        pareto = zeros(0, 2);

        tokens = regexp(value, '\(([\d.]+),\s*([\d.]+)\)', 'tokens');
        for j = 1:length(tokens)
          pareto(end + 1, 1:2) = [ str2num(tokens{j}{1}), str2num(tokens{j}{2}) ];
        end

        if L0 == 0 || E0 == 0
          error('The input file is invalid');
        end

        [ dummy, I ] = sort(pareto(:, 1));

        lifetime = pareto(I, 1) / L0;
        energy = pareto(I, 2) / E0;

        [ lifetime, energy ] = Utils.extractDominant(lifetime, energy);

        if nargin > 1 && specific == i
          Utils.drawPareto(lifetime, energy);
        end

        mnL = min(lifetime);
        mxL = max(lifetime);
        mnE = min(energy);
        mxE = max(energy);

        Lifetime(end + 1, 1:2) = [ mnL, mxL ];
        Energy(end + 1, 1:2) = [ mnE, mxE ];
        deltaL(end + 1) = (mxL - mnL) / mnL * 100;
        deltaE(end + 1) = (mxE - mnE) / mnE * 100;

        fprintf('%5d%15.2f%15.2f%15.2f%15.2f%15.2f%15.2f\n', i, ...
          mnL, mxL, mnE, mxE, deltaL(end), deltaE(end));

        S(end + 1, 1:N) = Utils.paretoSpline(lifetime, energy, N);

        d = (mxL - mnL) / (N - 1);
        x = mnL:d:mxL;

        L0 = 0;
        E0 = 0;
      end

      str = fgetl(fid);
    end

    fclose(fid);
  end

  mnL = mean(Lifetime(:, 1));
  mxL = mean(Lifetime(:, 2));
  mnE = mean(Energy(:, 1));
  mxE = mean(Energy(:, 2));

  fprintf('--\n');
  fprintf('%5s%15.2f%15.2f%15.2f%15.2f%15.2f%15.2f\n', 'avg', ...
    mnL, mxL, mnE, mxE, mean(deltaL), mean(deltaE));

  d = (mxL - mnL) / (N - 1);
  x = mnL:d:mxL;

  Utils.drawPareto(x, mean(S, 1), false);

  legend('Average Pareto Front');
end
