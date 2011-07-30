function time = stopTimer()
  global timerNames
  global timerTimes

  name = timerNames{end};
  time = toc(timerTimes{end});

  timerNames(end) = [];
  timerTimes(end) = [];

  fprintf('%-30s: %f s\n', name, time);
end
