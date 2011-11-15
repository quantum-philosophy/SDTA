function showEvolutionStats(file, full)
  if nargin < 2, full = false;

  files = dir(file);
  count = length(files);

  prefix = regexp(file, '(.*)/[^/]+$', 'tokens');
  prefix = prefix{1};
  prefix = prefix{1};

  generations = [];
  evaluations = [];
  deadline_misses = [];
  improvement = [];
  assessed_improvement = [];
  lifetime = [];
  time = [];

  for i = 1:count
    file = sprintf('%s/%s', prefix, files(i).name);
    fid = fopen(file);

    line = fgetl(fid);
    while ischar(line)
      tokens = regexp(line, '^\s*(\w[\w\s]+):\s*(.*)$', 'tokens');

      if ~isempty(tokens)
        name = tokens{1}{1};
        value = tokens{1}{2};

        switch (name)
        case 'Generations'
          generations(end + 1) = str2num(value);
        case 'Evaluations'
          evaluations(end + 1) = str2num(value);
        case 'Deadline misses'
          tokens = regexp(value, '([\d.]+)\s*(\(.*\))?', 'tokens');
          deadline_misses(end + 1) = str2num(tokens{1}{1});
        case 'Best lifetime'
          tokens = regexp(value, '\(([^,]+), .*', 'tokens');
          if isempty(tokens)
            lifetime(end + 1) = str2num(value);
          else
            lifetime(end + 1) = str2num(tokens{1}{1});
          end
        case 'Improvement'
          tokens = regexp(value, '([^%]+)%.*\s+(-?[^%]+)%.*', 'tokens');
          improvement = [ ...
            improvement; ...
            str2num(tokens{1}{1}) str2num(tokens{1}{2}) ...
          ];
        case 'Assessed improvement'
          tokens = regexp(value, '([^%]+)%.*\s+(-?[^%]+)%.*', 'tokens');
          assessed_improvement = [ ...
            assessed_improvement; ...
            str2num(tokens{1}{1}) str2num(tokens{1}{2}) ...
          ];
        case 'Time elapsed'
          tokens = regexp(value, '\s*([\d.]+\s*)', 'tokens');
          time(end + 1) = str2num(tokens{1}{1});
        end
      end

      line = fgetl(fid);
    end

    fclose(fid);
  end

  count = length(generations);

  if length(assessed_improvement) == 0
    assessed_improvement = zeros(size(improvement));
  end

  improvement = 1 + improvement / 100;
  assessed_improvement = 1 + assessed_improvement / 100;

  fprintf('%10s%15s%15s%15s%15s%15s%15s%15s%15s\n', ...
    'No', 'Generations', 'Evaluations', 'Deadline', ...
    'Lifetime', '+LT, x', '+LTa, x', '+E, x', 'Time, m');
  for i = 1:count
    fprintf('%10d%15d%15d%15d%15.2f%15.2f%15.2f%15.2e%15.2f\n', ...
      i, generations(i), evaluations(i), deadline_misses(i), ...
      lifetime(i), improvement(i, 1), assessed_improvement(i, 1), ...
      improvement(i, 2), time(i));
  end

  fprintf('\n');

  fprintf('%10s%15d%15d%15d%15.2f%15.2f%15.2f%15.2e%15.2f\n', 'min', ...
    min(generations), min(evaluations), min(deadline_misses), ...
    min(lifetime), min(improvement(:, 1)), min(assessed_improvement(:, 1)), ...
    min(improvement(:, 2)), min(time));

  fprintf('%10s%15d%15d%15d%15.2f%15.2f%15.2f%15.2e%15.2f\n', 'max', ...
    max(generations), max(evaluations), max(deadline_misses), ...
    max(lifetime), max(improvement(:, 1)), max(assessed_improvement(:, 1)), ...
    max(improvement(:, 2)), max(time));

  fprintf('%10s%15d%15d%15d%15.2f%15.2f%15.2f%15.2e%15.2f\n', 'mean', ...
    round(mean(generations)), round(mean(evaluations)), ...
    round(mean(deadline_misses)), mean(lifetime), ...
    mean(improvement(:, 1)), mean(assessed_improvement(:, 1)), ...
    mean(improvement(:, 2)), mean(time));

  if count > 1
    figure;
    Utils.drawProgress('Improvement, %', improvement(:, 1));
  end
end
