function profile = calculateProfile(graph, startTime, execTime, pes, mapping)
  cores = length(pes);

  taskPower = Power.calculateDynamic(graph, pes, mapping);
  finishTime = startTime + execTime;

  profile = zeros(0, cores);

  timeStep = Constants.samplingInterval;
  totalTime = max(finishTime);

  % ATTENTION: What should we do about this mismatch?
  steps = floor(totalTime / timeStep);
  if steps * timeStep < totalTime, steps = steps + 1; end

  for i = 1:cores
    % Find all tasks for this core
    ids = find(mapping == i);
    tasks = length(ids);

    if tasks == 0, continue; end

    % Sort them according to their start times
    [ dummy, I ] = sort(startTime(ids));
    ids = ids(I);

    for id = ids
      s = floor(startTime(id) / timeStep) + 1;
      % NOTE: Here without +1 to eliminate successor and predecessor
      % are being running at the same time
      e = floor(finishTime(id) / timeStep);
      profile(s:e, i) = taskPower(id);
    end
  end
end
