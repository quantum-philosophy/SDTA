function dumpField(file, name, field)
  if islogical(field)
    format = '%d';
  elseif isinteger(field)
    format = '%d';
  elseif isreal(field)
    format = '%e';
  else
    error('An unknown type.');
  end

  [ rows, cols ] = size(field);
  fprintf(file, '@%s (%d x %d)\n', name, rows, cols);

  for i = 1:rows
    for j = 1:cols
      if j < cols
        fprintf(file, [ format, '\t' ], field(i, j));
      else
        fprintf(file, [ format, '\n' ], field(i, j));
      end
    end
  end

  fprintf(file, '\n');
end
