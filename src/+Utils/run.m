function [ code, result ] = run(name, varargin)
  arguments = '';
  for arg = varargin, value = arg{1};
    if isnumeric(value), value = num2str(value); end
    arguments = [ arguments, ' ', value ];
  end

  command = sprintf('%s %s', Utils.path(name), arguments);

  Utils.startTimer('Execute: %s\n', command);
  [ code, result ] = system(command);
  Utils.stopTimer();
end
