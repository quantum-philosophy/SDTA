function time = stopTimer()
  global timerNames
  global timerTimes

  name = timerNames{end};
  time = toc(timerTimes{end});

  timerNames(end) = [];
  timerTimes(end) = [];

  if ~isempty(name), fprintf('%-50s: %f s\n', name, time); end
end
