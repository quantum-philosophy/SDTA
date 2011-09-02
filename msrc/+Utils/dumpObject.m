function dumpObject(object, filename)
  file = fopen(filename, 'w');

  names = fieldnames(object);

  for i = 1:length(names)
    name = names{i};
    Utils.dumpField(file, name, object.(name));
  end

  fclose(file);
end
