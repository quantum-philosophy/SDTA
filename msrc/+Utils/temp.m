function fullpath = temp(file)
  if file(1) == '#', file = file(2:end); end
  fullpath = [ Constants.workingDirectory, '/', file, '_temp' ];
end
