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
          tokens = regexp(value, '([^%]+)%.*', 'tokens');
          improvement(end + 1) = str2num(tokens{1}{1});
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

  fprintf('%10s%15s%15s%15s%15s%15s%15s\n', ...
    'No', 'Generations', 'Evaluations', 'Deadline', 'Lifetime', '+, %', 'Time, m');
  for i = 1:count
    fprintf('%10d%15d%15d%15d%15.2f%15.2f%15.2f\n', ...
      i, generations(i), evaluations(i), deadline_misses(i), ...
      lifetime(i), improvement(i), time(i));
  end

  fprintf('\n');

  fprintf('%10s%15d%15d%15d%15.2f%15.2f%15.2f\n', 'min', ...
    min(generations), min(evaluations), min(deadline_misses), ...
    min(lifetime), min(improvement), min(time));

  fprintf('%10s%15d%15d%15d%15.2f%15.2f%15.2f\n', 'max', ...
    max(generations), max(evaluations), max(deadline_misses), ...
    max(lifetime), max(improvement), max(time));

  fprintf('%10s%15d%15d%15d%15.2f%15.2f%15.2f\n', 'mean', ...
    round(mean(generations)), round(mean(evaluations)), ...
    round(mean(deadline_misses)), mean(lifetime), ...
    mean(improvement), mean(time));

  if count > 1
    figure;
    Utils.drawProgress('Improvement, %', improvement);
  end
end
