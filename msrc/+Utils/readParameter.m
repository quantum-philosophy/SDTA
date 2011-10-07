function parameter = readParameter(file, name)
  fid = fopen(file, 'r');

  expression = [ '^\s*', name, '\s+(.*)$' ];

  line = fgetl(fid);
  while ischar(line)
    tokens = regexp(line, expression, 'tokens');

    if ~isempty(tokens)
      parameter = str2num(tokens{1}{1});
      break;
    end

    line = fgetl(fid);
  end

  fclose(fid);
end
