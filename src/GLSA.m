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

  properties (Access = private)
    graph             % Task graph
    taskCount         % Number of tasks (just a shortcut)
    evaluateSchedule  % Evaluate a schedule
    options           % Options for GA
    scheduler         % List scheduler
  end

  methods
    function glsa = GLSA(varargin)
      % Configure GA
      options = gaoptimset;
      options.EliteCount = floor(glsa.generationalGap * glsa.populationSize);
      options.StallGenLimit = glsa.generationalStall;
      options.TolFun = glsa.generationalTolerance;
      options.CrossoverFcn = @crossovertwopoint;
      options.CreationFcn = @glsa.create;
      options.MutationFcn = @glsa.mutate;

      glsa.options = options;
      glsa.scheduler = LS();

      if nargin > 0, glsa.process(varargin{:}); end
    end

    function [ solution, fitness, flag ] = process(glsa, graph, evaluateSchedule)
      glsa.graph = graph;
      glsa.taskCount = length(graph.tasks);
      glsa.evaluateSchedule = evaluateSchedule;

      [ solution, fitness, flag ] = ga(@glsa.evaluate, ...
        glsa.taskCount, [], [], [], [], [], [], [], glsa.options);
    end
  end

  methods (Access = private)
    function population = create(glsa, varargin)
      psize = glsa.populationSize;
      tcount = glsa.taskCount;

      population = zeros(psize, tcount);
      half = floor(psize / 2);

      % Half of the population is generated based on the mobility
      % of the tasks, and the rest is randomly generated with values
      % between the lowest and highest mobility

      % Use ordinal numbers instead of plain mobility
      mobility = Utils.mapToVector(glsa.graph.tasks, 'deadline');
      [ mobility, I ] = sort(mobility);

      % The first half
      for i = 1:half, population(i, I) = 1:tcount; end

      % The second half
      population((half + 1):end, :) = randi(tcount, psize - half, tcount);
    end

    function children = mutate(glsa, parents, options, genomeLength, ...
      fitnessFunc, state, thisScore, thisPopulation)

      ccount = length(parents);
      tcount = glsa.taskCount;

      % To mutate or not to mutate? That is the question...
      % The probability to mutate should not be less than 15%
      mprob = max(0.15, 1 / exp(state.Generation * 0.05));

      children = zeros(ccount, tcount);

      for i = 1:ccount
        child = thisPopulation(parents(i), :);
        mutationPoints = find(rand(1, tcount) < mprob);
        child(mutationPoints) = randi(tcount, 1, length(mutationPoints));
        children(i, :) = child;
      end
    end

    function fitness = evaluate(glsa, priority)
      schedule = glsa.scheduler.process(glsa.graph, priority);
      fitness = glsa.evaluateSchedule(schedule);
    end
  end
end
