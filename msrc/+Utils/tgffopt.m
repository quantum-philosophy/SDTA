function tgffopt(opt, tgff, sys)
  root = Constants.thisDirectory;

  name = regexprep(opt, '.tgffopt', '');

  command = [ root, '/../build/vendor/tgff', ' ', name ];
  [ status, result ] = system(command);
  if status > 0, error('Cannot execute TGFF'); end

  command = [ root, '/../build/tools/system', ' ', tgff, ' > ', sys ];
  [ status, result ] = system(command);
  if status > 0, error('Cannot execute SYSTEM'); end
end
