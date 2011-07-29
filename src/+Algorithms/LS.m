function schedule = LS(graph, priority)
  tasks = length(graph.tasks);

  if nargin < 3
    priority = zeros(0, 0);
    for task = graph.tasks, priority(end + 1) = task{1}.deadline; end
  end

  pool = graph.getStartPoints();
  done = zeros(1, tasks);
  done(pool) = 1;

  schedule = zeros(0, 0);

  while ~isempty(pool)
    % Find the most urgent task (the lower priority, more urgent)
    [ dummy, index ] = min(priority(pool));
    id = pool(index);

    % Exclude the task
    pool(index) = [];

    % Append to the schedule
    schedule(end + 1) = id;

    % Append new tasks, but only ready ones, and ensure absence
    % of repetitions
    nids = graph.taskIndexesFrom{id};
    for nid = nids
      if done(nid), continue; end
      ins = graph.taskIndexesTo{nid};
      nin = length(ins);
      if nin == 1 || nin == length(intersect(ins, find(done)))
        done(nid) = 1;
        pool(end + 1) = nid;
      end
    end
  end
end
