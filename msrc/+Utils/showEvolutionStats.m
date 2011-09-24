function showEvolutionStats(file)
  fid = fopen(file);

  generations = [];
  evaluations = [];
  deadline_misses = [];
  improvement = [];
  lifetime = [];
  time = [];

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
        deadline_misses(end + 1) = str2num(value);
      case 'Best lifetime'
        lifetime(end + 1) = str2num(value);
      case 'Improvement'
        tokens = regexp(value, '([^%]+)%.*', 'tokens');
        improvement(end + 1) = str2num(tokens{1}{1});
      case 'Time elapsed'
        time(end + 1) = str2num(value);
      end
    end

    line = fgetl(fid);
  end

  fclose(fid);

  count = length(generations);

  fprintf('%10s%15s%15s%15s%15s%15s%15s\n', ...
    'No', 'Generations', 'Evaluations', 'Deadline', 'Lifetime', '+, %', 'Time, m');
  for i = 1:count
    fprintf('%10d%15d%15d%15d%15.2f%15.2f%15.2f\n', ...
      i, generations(i), evaluations(i), deadline_misses(i), ...
      lifetime(i), improvement(i), time(i)/60);
  end

  fprintf('\n');

  fprintf('%10s%15d%15d%15d%15.2f%15.2f%15.2f\n', 'min', ...
    min(generations), min(evaluations), min(deadline_misses), ...
    min(lifetime), min(improvement), min(time/60));

  fprintf('%10s%15d%15d%15d%15.2f%15.2f%15.2f\n', 'max', ...
    max(generations), max(evaluations), max(deadline_misses), ...
    max(lifetime), max(improvement), max(time/60));

  fprintf('%10s%15d%15d%15d%15.2f%15.2f%15.2f\n', 'mean', ...
    round(mean(generations)), round(mean(evaluations)), ...
    round(mean(deadline_misses)), mean(lifetime), ...
    mean(improvement), mean(time/60));

  figure;
  subplot(2, 3, 1);
  Utils.drawProgress('Generations', generations);

  subplot(2, 3, 2);
  Utils.drawProgress('Evaluations', evaluations);

  subplot(2, 3, 3);
  Utils.drawProgress('Deadline misses', deadline_misses);

  subplot(2, 3, 4);
  Utils.drawProgress('Lifetime', lifetime);

  subplot(2, 3, 5);
  Utils.drawProgress('Improvement', improvement);

  subplot(2, 3, 6);
  Utils.drawProgress('Time', time);
end
