classdef GLSA < handle
  properties (Constant)
    % Size of the solution pool
    populationSize = 25;

    % Half of individuals in the solution pool survive
    generationalGap = 0.5;

    % Stop criterion: there is no considerable improvement
    % through several generations
    generationalStall = 10;
    generationalTolerance = 1e-6;
  end

  properties (SetAccess = private)
    taskCount     % Number of tasks
    tasks         % Instances of tasks

    mobility      % Mobility of the tasks
    minMobility   % Maximal mobility
    maxMobility   % Minimal mobility

    options       % Options for GA
  end

  methods
    function model = GLSA()
      taskCount = 5;

      % Generate tasks
      tasks = {};
      for i = 1:taskCount
        tasks{i} = Task();
      end

      % Configure GA
      options = gaoptimset;
      options.EliteCount = floor(model.generationalGap * model.populationSize);
      options.StallGenLimit = generationalStall;
      options.TolFun = generationalTolerance;
      options.CrossoverFcn = @crossovertwopoint;
      options.CreateFcn = @model.create;
      options.MutationFcn = @model.mutate;

      model.taskCount = taskCount;
      model.tasks = tasks;

      model.mobility = map(tasks, 'mobility');
      model.minMobility = max(model.mobility);
      model.maxMobility = min(model.mobility);

      model.options = options;
    end

    function population = create(model, varargin)
      psize = model.populationSize;
      tcount = model.taskCount;

      population = zeros(psize, tcount);
      half = floor(psize / 2);

      mobility = model.mobility;
      maxm = model.maxMobility;
      minm = model.minMobility;

      % Half of the population is generated based on the mobility
      % of the tasks
      for i = 1:half
        population(i, :) = mobility;
      end

      % The rest is randomly generated with values between
      % the lowest and highest mobility
      for i = (half + 1):psize
        population(i, :) = minm + (maxm - minm) * rand(1, tcount);
      end
    end

    function children = mutate(model, parents, options, d1, state, d2, thisPopulation)
      ccount = length(parents);
      tcount = model.taskCount;

      % To mutate or not to mutate? That is the question...
      % The probability to mutate should not be less than 15%
      mprob = max(0.15, 1 / exp(state.Generation * 0.05))

      children = zeros(ccount, tcount);

      maxm = model.maxMobility;
      minm = model.minMobility;
      span = maxm - minm;

      for i = 1:ccount
        child = thisPopulation(parents(i), :);
        mutationPoints = find(rand(1, tcount) < mprob);
        child(mutationPoints) = minm + span * rand(1, length(mutationPoints));
        children(i, :) = child
      end
    end

    function fitness = evaluate(model, genome)
      % Create a list schedule
      list = model.schedule(genome);
    end

    function list = schedule(model, priority)
    end
  end
end
