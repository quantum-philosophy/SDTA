function showEvolutionStats(file, remove)
  if nargin < 2, remove = []; end

  files = dir(file);
  count = length(files);

  prefix = regexp(file, '(.*)/[^/]+$', 'tokens');
  prefix = prefix{1};
  prefix = prefix{1};

  G = 1;
  E = 2;
  D = 3;
  R = 4;
  IL = 5;
  IE = 6;
  AL = 7;
  AE = 8;
  T = 9;

  Stats = zeros(0, 9);

  for i = 1:count
    file = sprintf('%s/%s', prefix, files(i).name);
    fid = fopen(file);

    stats = zeros(0, size(Stats, 2));

    line = fgetl(fid);
    while ischar(line)
      tokens = regexp(line, '^\s*(\w[\w\s]+):\s*(.*)$', 'tokens');

      if ~isempty(tokens)
        name = tokens{1}{1};
        value = tokens{1}{2};

        switch (name)
        case 'Generations'
          stats(end + 1, :) = zeros(1, size(stats, 2));
          stats(end, G) = str2num(value);
        case 'Evaluations'
          stats(end, E) = str2num(value);
        case 'Deadline misses'
          tokens = regexp(value, '([\d.]+)\s*(\(.*\))?', 'tokens');
          stats(end, D) = str2num(tokens{1}{1});
        case 'Temperature runaways'
          tokens = regexp(value, '([\d.]+)\s*(\(.*\))?', 'tokens');
          stats(end, R) = str2num(tokens{1}{1});
        case 'Improvement'
          tokens = regexp(value, '(.+) lifetime with (.+) energy', 'tokens');
          stats(end, IL) = str2num(tokens{1}{1});
          stats(end, IE) = str2num(tokens{1}{2});
        case 'Assessed improvement'
          tokens = regexp(value, '(.+) lifetime with (.+) energy', 'tokens');
          stats(end, AL) = str2num(tokens{1}{1});
          stats(end, AE) = str2num(tokens{1}{2});
        case 'Time elapsed'
          tokens = regexp(value, '\s*([\d.]+\s*)', 'tokens');
          stats(end, T) = str2num(tokens{1}{1});
        end
      end

      line = fgetl(fid);
    end

    fclose(fid);

    if size(stats, 1) == 0
      continue;
    end

    [ dummy, I ] = sort(stats(:, IL), 1, 'descend');

    Stats(end + 1, :) = stats(I(1), :);
  end

  count = size(Stats, 1);

  fprintf('%10s%15s%15s%15s%15s%15s%15s%15s%15s\n', ...
    'No', 'Generations', 'Evaluations', 'Deadline', 'Runaways', ...
    '+LT, x', '+LTa, x', '+E, x', 'Time, s');

  for i = 1:count
    if any(remove == i)
      fprintf('%10s%15d%15d%15d%15d%15.2f%15.2f%15.2f%15.4f\n', ...
        'X', Stats(i, G), Stats(i, E), Stats(i, D), Stats(i, R), ...
        Stats(i, IL), Stats(i, AL), Stats(i, IE), Stats(i, T));
    else
      fprintf('%10d%15d%15d%15d%15d%15.2f%15.2f%15.2f%15.4f\n', ...
        i, Stats(i, G), Stats(i, E), Stats(i, D), Stats(i, R), ...
        Stats(i, IL), Stats(i, AL), Stats(i, IE), Stats(i, T));
    end
  end

  fprintf('\n');

  Stats(remove, :) = [];

  fprintf('%10s%15d%15d%15d%15d%15.2f%15.2f%15.2f%15.2f\n', 'min', ...
    min(Stats(:, G)),  min(Stats(:, E)),  min(Stats(:, D)), ...
    min(Stats(:, R)),  min(Stats(:, IL)), min(Stats(:, AL)), ...
    min(Stats(:, IE)), min(Stats(:, T)));

  fprintf('%10s%15d%15d%15d%15d%15.2f%15.2f%15.2f%15.2f\n', 'max', ...
    max(Stats(:, G)),  max(Stats(:, E)),  max(Stats(:, D)), ...
    max(Stats(:, R)),  max(Stats(:, IL)), max(Stats(:, AL)), ...
    max(Stats(:, IE)), max(Stats(:, T)));

  fprintf('%10s%15.0f%15.0f%15.0f%15.0f%15.2f%15.2f%15.2f%15.2f\n', 'mean', ...
    mean(Stats(:, G)),  mean(Stats(:, E)),  mean(Stats(:, D)), ...
    mean(Stats(:, R)),  mean(Stats(:, IL)), mean(Stats(:, AL)), ...
    mean(Stats(:, IE)), mean(Stats(:, T)));

  fprintf('%10s%15s%15s%15.2f%15s%15s%15s%15s%15s\n', 'percent', ...
    '',  '', mean(Stats(:, D)) / mean(Stats(:, E)) * 100, '', '', '', '', '');

  if count > 1
    figure;
    Utils.drawProgress('Improvement, %', Stats(:, IL));
  end
end
