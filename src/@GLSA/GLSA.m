classdef GLSA < handle
  properties (Constant)
    % Stop criteria
    generationalLimit = 500; % generations

    % How many individuals of the start population are generated
    % with the same chromosomes based on the initial mobility?
    mobilityCreationFactor = 0.4;

    % Size of the solution pool
    populationSize = 25; % individuals

    % Fraction of individuals who survive
    generationalGap = 0.4;

    % How much to crossover and mutate?
    % (excluding the elite specified by generationalGap)
    crossoverFraction = 0.5;

    % Minimal probability for mutation
    minimalMutationProbability = 0.5;

    % Maximal number of iterations for the leakage loop
    maxLeakageIterations = 10;

    % Tolerance for the leakage loop
    leakageTolerance = 0.01; % K
  end

  properties (SetAccess = private)
    options

    graph
    hotspot
    deadline

    % Leakage
    vdd
    ngate

    % Creation
    mobility
    maxMobility
    minMobility

    evaluations

    cache

    % Drawing and progress
    drawing
    currentLine
    bar
  end

  methods
    function glsa = GLSA()
      % Configure GA
      options = gaoptimset;

      % We know when to stop ourselves
      options.Generations = Inf;
      options.StallGenLimit = Inf;
      options.TolFun = 0;

      options.PopulationSize = glsa.populationSize;
      options.EliteCount = floor(glsa.generationalGap * glsa.populationSize);
      options.CrossoverFraction = glsa.crossoverFraction;

      options.CrossoverFcn = @glsa.crossover; % crossoverRealRandom;
      options.CreationFcn = @glsa.create;
      options.MutationFcn = @glsa.mutate; % mutateNothing;
      options.OutputFcns = [ @glsa.output ];

      glsa.options = options;
    end

    function [ solution, fitness, output ] = solve(...
      glsa, graph, hotspot, draw)

      if nargin < 4, draw = false; end

      glsa.graph = graph;
      glsa.hotspot = hotspot;

      glsa.vdd = zeros(0, 0);
      glsa.ngate = zeros(0, 0);

      for pe = graph.pes, pe = pe{1};
        glsa.vdd(end + 1) = pe.voltage;
        glsa.ngate(end + 1) = pe.ngate;
      end

      chromosomeLength = length(graph.tasks);

      % Assume that the initial deadline is the desired one,
      % and that the current mobility is suitable for the initial
      % population. We should meet this deadline.
      glsa.deadline = graph.deadline;

      % Collect mobility
      glsa.mobility = zeros(1, chromosomeLength);
      for i = 1:chromosomeLength
        glsa.mobility(i) = graph.tasks{i}.mobility;
      end
      glsa.maxMobility = max(glsa.mobility);
      glsa.minMobility = min(glsa.mobility);

      glsa.cache = containers.Map('KeyType', 'char', 'ValueType', 'any');

      % Reset
      glsa.evaluations = 0;
      glsa.bar = waitbar(0, 'Genetic List Scheduling Algorithm');

      if draw, glsa.initializeDrawing(); end

      [ solution, fitness, exitflag, output, population, score ] = ...
        gamultiobj(@glsa.evaluate, chromosomeLength, ...
          [], [], [], [], [], [], glsa.options);

      delete(glsa.bar);
    end
  end

  methods (Access = private)
    function population = create(glsa, chromosomeLength, fitnessFcn, options)
      psize = glsa.populationSize;

      population = zeros(psize, chromosomeLength);
      half = floor(glsa.mobilityCreationFactor * psize);

      % One part of the population is generated based on the mobility
      % of the tasks, and the rest is randomly generated with values
      % between the lowest and highest mobility

      % The first part
      for i = 1:half, population(i, :) = glsa.mobility; end

      % The second part
      population((half + 1):psize, :) = ...
        glsa.rand(psize - half, chromosomeLength);
    end

    function children = crossover(glsa, parents, options, genomeLength, ...
      FitnessFcn, dummy, thisPopulation)

      ccount = floor(length(parents) / 2);
      children = zeros(ccount, genomeLength);

      index = 1;

      for i = 1:ccount
        % Parents
        father = thisPopulation(parents(index), :);
        index = index + 1;
        mother = thisPopulation(parents(index), :);
        index = index + 1;

        % Points
        mp  = length(father) - 1;
        xp1 = ceil(mp * rand);
        xp2 = ceil(mp * rand);
        while xp2 == xp1
          xp2 = ceil(mp * rand);
        end

        if xp1 < xp2
          left = xp1;
          right = xp2;
        else
          left = xp2;
          right = xp1;
          swap = father;
          father = mother;
          mother = swap;
        end

        children(i, :) = ...
          [ father(1:left), mother((left + 1):right), father((right + 1):end) ];
      end
    end

    function children = crossoverRealRandom(glsa, parents, options, genomeLength, ...
      FitnessFcn, dummy, thisPopulation)

      ccount = floor(length(parents) / 2);
      children = zeros(ccount, genomeLength);

      index = 1;

      for i = 1:ccount
        % Parents
        father = thisPopulation(parents(index), :);
        index = index + 1;
        mother = thisPopulation(parents(index), :);
        index = index + 1;

        % Boundaries
        mx = max(father, mother);
        mn = min(father, mother);

        children(i, :) = mn + (mx - mn) .* rand(1, genomeLength);
      end
    end

    function children = mutate(glsa, parents, options, chromosomeLength, ...
      fitnessFunc, state, thisScore, thisPopulation)

      ccount = length(parents);

      % To mutate or not to mutate? That is the question...
      % The probability to mutate should not be less than 15%
      mprob = max(glsa.minimalMutationProbability, ...
        1 / exp(state.Generation * 0.05));

      children = zeros(ccount, chromosomeLength);

      for i = 1:ccount
        child = thisPopulation(parents(i), :);
        mutationPoints = find(rand(1, chromosomeLength) < mprob);
        child(mutationPoints) = glsa.rand(1, length(mutationPoints));
        children(i, :) = child;
      end
    end

    function children = mutateNothing(glsa, parents, options, chromosomeLength, ...
      fitnessFunc, state, thisScore, thisPopulation)

      children = parents;
    end

    function fitness = evaluate(glsa, chromosome)
      glsa.evaluations = glsa.evaluations + 1;
      waitbar(mod(glsa.evaluations, 10) / 10, glsa.bar, ...
        [ 'Evaluation #' num2str(glsa.evaluations) ]);

      key = Utils.mMD5(chromosome);

      if glsa.cache.isKey(key)
        fitness = glsa.cache(key);
        return;
      end

      % Make a new schedule
      LS.schedule(glsa.graph, chromosome);

      if glsa.graph.duration > glsa.deadline
        % Respect the deadline!
        fitness = [ Inf, Inf ];
      else
        % The graph is rescheduled now, obtain the dynamic power profile
        dynamicPowerProfile = Power.calculateDynamicProfile(glsa.graph);

        % Get the temperature curve
        [ T, it, totalPowerProfile ] = ...
          glsa.hotspot.solveCondensedEquationWithLeakage( ...
            dynamicPowerProfile, glsa.vdd, glsa.ngate, ...
            glsa.leakageTolerance, glsa.maxLeakageIterations);

        % We want to prolong aging
        aging = -min(Lifetime.predict(T));
        % ... and we want to keep the energy low
        energy = sum(sum(totalPowerProfile)) * Constants.samplingInterval;

        fitness = [ aging, energy ];
      end

      % Cache it!
      glsa.cache(key) = fitness;
    end

    function [ state, options, changed ] = output(glsa, ...
      options, state, flag, interval)

      changed = false;

      if glsa.drawing, glsa.drawGeneration(state); end

      no = state.Generation;

      if no >= glsa.generationalLimit
        state.StopFlag = 'Exceed the number of generations';
      end
    end

    function mobility = rand(glsa, rows, cols)
      mobility = glsa.minMobility + ...
        (glsa.maxMobility - glsa.minMobility) * rand(rows, cols);
    end

    function initializeDrawing(glsa)
      glsa.drawing = figure;
      glsa.currentLine = [];
      title('Energy and Aging');
      ylabel('Energy consumption, J');
      xlabel('Lifetime duration, time units');
      grid on;
    end

    function drawGeneration(glsa, state)
      if state.Generation == 0, return; end

      index = find(state.Rank == 1);
      aging = -state.Score(index, 1);
      energy = state.Score(index, 2);

      figure(glsa.drawing);
      title([ 'Energy and Aging (generation ', num2str(state.Generation), ')' ]);

      if ~isempty(glsa.currentLine)
        set(glsa.currentLine, ...
          'Color', 'k', 'Line', 'none', 'Marker', 'x');
      end

      glsa.currentLine = line(aging, energy, ...
        'Color', 'r', 'Line', 'none', 'Marker', 'o');
    end
  end

  methods (Static)
    function N = calculatePopulationSize(L, P, S)
      if nargin < 3, S = L; end

      % +L+ is a length of a chromosome, a number of genes
      % +S+ is a number of different states of a gene
      % +P+ is a desired probability to ensure diversity

      N = ceil(1 - log(1 - P^(1 / L)) / log(S));
    end

    function P = calculateMinimalMutationProbability(L, N, S, K)
      if nargin < 3, S = L; end
      if nargin < 4, K = 10; end

      % +L+ is a length of a chromosome, a number of genes
      % +S+ is a number of different states of a gene
      % +N+ is a population size
      % +K+ is an order of insurance that a gene will mutate
      % in case it does not take all possible states in the
      % given population

      P = L * (1 - (1 - K * S^(1 - N))^(1 / N));
    end
  end
end
