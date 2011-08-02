function startTimer(varargin)
  global timerNames
  global timerTimes

  if nargin > 0, timerNames{end + 1} = sprintf(varargin{:});
  else timerNames{end + 1} = [];
  end

  timerTimes{end + 1} = tic;
end
