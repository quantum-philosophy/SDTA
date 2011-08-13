classdef GLSA < handle
  properties (Constant)
    % Size of the solution pool
    populationSize = 25;

    % Half of individuals in the solution pool survive
    generationalGap = 0.5;

    % Stop criterion: there is no considerable improvement
    % through several generations
    generationalStall = 10;
    generationalTolerance = 1;

    % Maximal number of iterations for the leakage loop
    maxLeakageIterations = 10;

    % Tolerance for the leakage loop
    leakageTolerance = 0.01; % K
  end

  properties (SetAccess = private)
    options

    graph
    thermalModel

    vdd
    ngate

    pause

    cache

    evaluationCount
    bar
  end

  methods
    function glsa = GLSA()
      % Configure GA
      options = gaoptimset;
      options.EliteCount = floor(glsa.generationalGap * glsa.populationSize);
      options.StallGenLimit = glsa.generationalStall;
      options.TolFun = glsa.generationalTolerance;
      options.CrossoverFcn = @crossovertwopoint;
      options.CreationFcn = @glsa.create;
      options.MutationFcn = @glsa.mutate;
      glsa.options = options;
    end

    function [ solution, fitness, flag ] = solve(glsa, graph, thermalModel, pause)
      if nargin < 4, glsa.pause = false; end

      glsa.cache = containers.Map('KeyType', 'char', 'ValueType', 'double');
      glsa.graph = graph;
      glsa.thermalModel = thermalModel;

      glsa.vdd = zeros(0, 0);
      glsa.ngate = zeros(0, 0);

      for pe = graph.pes, pe = pe{1};
        glsa.vdd(end + 1) = pe.voltage;
        glsa.ngate(end + 1) = pe.ngate;
      end

      glsa.evaluationCount = 0;
      glsa.bar = waitbar(0, 'Genetic List Scheduling Algorithm');

      [ solution, fitness, flag ] = ga(@glsa.evaluate, ...
        length(graph.tasks), [], [], [], [], [], [], [], glsa.options);

      waitbar(1, glsa.bar);
    end
  end

  methods (Access = private)
    function population = create(glsa, chromosomeLength, fitnessFcn, options)
      psize = glsa.populationSize;

      population = zeros(psize, chromosomeLength);
      half = floor(psize / 2);

      % Half of the population is generated based on the mobility
      % of the tasks, and the rest is randomly generated with values
      % between the lowest and highest mobility

      % Use ordinal numbers instead of plain mobility
      deadlines = zeros(0);
      for task = glsa.graph.tasks, deadlines(end + 1) = task{1}.deadline; end
      [ dummy, I ] = sort(deadlines);

      % The first half
      for i = 1:half
        population(i, I) = 1:chromosomeLength;
      end

      % The second half
      for i = (half + 1):psize
        population(i, :) = randperm(chromosomeLength);
      end
    end

    function children = mutate(glsa, parents, options, chromosomeLength, ...
      fitnessFunc, state, thisScore, thisPopulation)

      ccount = length(parents);

      % To mutate or not to mutate? That is the question...
      % The probability to mutate should not be less than 15%
      mprob = max(0.15, 1 / exp(state.Generation * 0.05));

      children = zeros(ccount, chromosomeLength);

      for i = 1:ccount
        child = thisPopulation(parents(i), :);
        mutationPoints = find(rand(1, chromosomeLength) < mprob);
        child(mutationPoints) = ...
          randi(chromosomeLength, 1, length(mutationPoints));
        children(i, :) = child;
      end
    end

    function fitness = evaluate(glsa, chromosome) % i.e. priority
      glsa.evaluationCount = glsa.evaluationCount + 1;
      waitbar(mod(glsa.evaluationCount, 10) / 10, glsa.bar, ...
        [ 'Evaluation #' num2str(glsa.evaluationCount) ]);

      key = Utils.mMD5(chromosome);

      if glsa.cache.isKey(key)
        fitness = glsa.cache(key);
      else
        % Make a new schedule
        LS.schedule(glsa.graph, chromosome);

        % The graph is rescheduled now, obtain the dynamic power profile
        dynamicPowerProfile = Power.calculateDynamicProfile(glsa.graph);

        % Get the temperature curve
        [ T, it ] = glsa.thermalModel.solveCondensedEquationWithLeakage( ...
          dynamicPowerProfile, glsa.vdd, glsa.ngate, ...
          glsa.leakageTolerance, glsa.maxLeakageIterations);

        if glsa.pause
          fitness = -min(Lifetime.predictAndDraw(T));
          glsa.pause = false;
          pause;
        else
          fitness = -min(Lifetime.predict(T));
        end

        % Cache it!
        glsa.cache(key) = fitness;
      end
    end
  end
end
