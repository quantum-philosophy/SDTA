function fullpath = path(file)
  if file(1) == '#'
    fullpath = [ pwd, '/', file(2:end) ];
  else
    fullpath = [ Constants.workingDirectory, '/', file ];
  end
end
