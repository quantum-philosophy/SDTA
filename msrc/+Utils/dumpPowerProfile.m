function dumpPowerProfile(file, power, names)
  steps = size(power, 1);
  cores = size(power, 2);

  if nargin < 3
    names = {};
    for i = 1:cores
      names{end + 1} = sprintf('core%d', i);
    end
  end

  f = fopen(file, 'w');

  for i = 1:cores
    fprintf(f, '%s\t', names{i});
  end
  fprintf(f, '\n');

  for i = 1:steps
    for j = 1:cores
      fprintf(f, '%f\t', power(i, j));
    end
    fprintf(f, '\n');
  end

  fclose(f);
end
