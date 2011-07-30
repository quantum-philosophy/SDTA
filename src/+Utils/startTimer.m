function startTimer(varargin)
  global timerNames
  global timerTimes

  timerNames{end + 1} = sprintf(varargin{:});
  timerTimes{end + 1} = tic;
end
