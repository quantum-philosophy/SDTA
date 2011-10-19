function parameter = writeParameter(file, ofile, name, value)
  if ~ischar(value), value = num2str(value); end
  param = sprintf('%s %s', name, value);

  lines = {};

  fid = fopen(file, 'r');

  expression = [ '^\s*', name, '\s+(.*)$' ];

  line = fgetl(fid);
  while ischar(line)
    tokens = regexp(line, expression, 'tokens');

    if isempty(tokens)
      lines{end + 1} = line;
    else
      lines{end + 1} = param;
    end

    line = fgetl(fid);
  end

  fclose(fid);

  fid = fopen(ofile, 'w');

  for i = 1:length(lines)
    fprintf(fid, '%s\n', lines{i});
  end

  fclose(fid);
end
