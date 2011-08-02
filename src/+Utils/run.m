function [ code, result ] = run(name, varargin)
  cd = false;

  if length(varargin) > 0 && islogical(varargin{end})
    cd = varargin{end};
    varargin(end) = [];
  end

  arguments = '';
  for arg = varargin, value = arg{1};
    if isnumeric(value), value = num2str(value); end
    arguments = [ arguments, ' ', value ];
  end

  if cd
    command = sprintf('cd %s && ./%s %s', ...
      Constants.workingDirectory, name, arguments);
  else
    command = sprintf('%s %s', Utils.path(name), arguments);
  end

  Utils.startTimer('Execute: %s', command);
  [ code, result ] = system(command);
  Utils.stopTimer();
end
