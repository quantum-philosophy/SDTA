function startTimer(name)
  global timerNames
  global timerTimes

  timerNames{end + 1} = name;
  timerTimes{end + 1} = tic;
end
