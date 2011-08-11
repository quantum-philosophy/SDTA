function profile = fitProfile(powerProfile, steps)
  currentSteps = size(powerProfile, 1);

  Utils.startTimer('Transform the power profile from %d to %d', ...
    currentSteps, steps);

  if steps < currentSteps
    profile = powerProfile(1:steps, :);
  elseif steps > currentSteps
    repeat = floor(steps / currentSteps);
    profile = zeros(0, 0);
    for i = 1:repeat
      profile = [ profile; powerProfile ];
    end
    rest = steps - repeat * currentSteps;
    profile = [ profile; powerProfile(1:rest, :) ];
  end

  Utils.stopTimer();
end
