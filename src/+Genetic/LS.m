classdef LS < handle
  methods (Static)
    function t = defaultTuning(varargin)
      t = struct();

      % Maximal number of iterations for the leakage loop
      t.maxLeakageIterations = 10;

      % Tolerance for the leakage loop
      t.leakageTolerance = 0.01; % K

      % Stop criteria
      t.generationLimit = 200;

      % How many individuals of the start population are generated
      % with the same chromosomes based on the initial mobility?
      t.mobilityCreationFactor = 0.5;

      % Size of the solution pool
      t.populationSize = 25; % individuals

      % How much to crossover and mutate?
      t.crossoverFraction = 0.8;

      % Minimal probability for mutation
      t.mutationProbability = 'max(0.15, 1 / exp(state.Generation * 0.05))';

      % Update!
      for i = 1:2:length(varargin)
        t.(varargin{i}) = varargin{i + 1};
      end
    end
  end

  properties (SetAccess = protected)
    options
    tuning

    solver
    additionalParams

    graph
    hotspot
    deadline

    chromosomeLength

    % Leakage
    vdd
    ngate

    % Creation
    mobility
    maxMobility
    minMobility

    % Stats
    evaluations

    % Drawing and progress
    drawing
    bar

    % Caching
    cache
    fitnessType
  end

  methods (Abstract, Access = protected)
    fitness = compute(chromosome)
    terminate = control(state)
    initializeDrawing()
    drawGeneration(state)
  end

  methods
    function ls = LS(graph, hotspot, tuning)
      if nargin < 3, tuning = ls.defaultTuning(); end

      % Tunning
      [ ls.options, ls.tuning ] = ls.tune(tuning);

      % Solver itself
      ls.solver = @ga;
      ls.additionalParams = cell(1, 7);

      ls.graph = graph;
      ls.hotspot = hotspot;

      ls.vdd = zeros(0, 0);
      ls.ngate = zeros(0, 0);

      for pe = graph.pes, pe = pe{1};
        ls.vdd(end + 1) = pe.voltage;
        ls.ngate(end + 1) = pe.ngate;
      end

      % Assume that the initial deadline is the desired one,
      % and that the current mobility is suitable for the initial
      % population. We should meet this deadline.
      ls.deadline = graph.deadline;

      % Collect mobility
      ls.chromosomeLength = length(graph.tasks);
      ls.mobility = zeros(1, ls.chromosomeLength);
      for i = 1:ls.chromosomeLength
        ls.mobility(i) = graph.tasks{i}.mobility;
      end
      ls.maxMobility = max(ls.mobility);
      ls.minMobility = min(ls.mobility);

      % Caching
      ls.fitnessType = 'double';
    end

    function [ solution, fitness, output ] = solve(ls, drawing)
      if nargin > 1
        grid(drawing, 'on');
        ls.drawing = drawing;
        ls.initializeDrawing();
      end

      % Reset
      ls.cache = containers.Map('KeyType', 'char', 'ValueType', ls.fitnessType);
      ls.evaluations = 0;

      if ls.drawing
        ls.bar = waitbar(0, 'Genetic List Scheduling Algorithm');
      end

      [ solution, fitness, exitflag, output ] = ls.solver(@ls.evaluate, ...
        ls.chromosomeLength, ls.additionalParams{:}, ls.options);

      if ls.drawing
        delete(ls.bar);
      end
    end
  end

  methods (Access = protected)
    function fitness = evaluate(ls, chromosome)
      ls.evaluations = ls.evaluations + 1;

      if ls.drawing
        waitbar(mod(ls.evaluations, 10) / 10, ls.bar, ...
          [ 'Evaluation #' num2str(ls.evaluations) ]);
      end

      key = Utils.mMD5(chromosome);

      if ls.cache.isKey(key)
        fitness = ls.cache(key);
        return;
      end

      fitness = ls.compute(chromosome);

      % Cache it!
      ls.cache(key) = fitness;
    end

    function population = create(ls, chromosomeLength, fitnessFcn, options)
      psize = ls.tuning.populationSize;

      population = zeros(psize, chromosomeLength);
      half = floor(ls.tuning.mobilityCreationFactor * psize);

      % One part of the population is generated based on the mobility
      % of the tasks, and the rest is randomly generated with values
      % between the lowest and highest mobility

      % The first part
      for i = 1:half, population(i, :) = ls.mobility; end

      % The second part
      population((half + 1):psize, :) = ...
        ls.rand(psize - half, chromosomeLength);
    end

    function expectation = rank(ls, scores, pcount)
      psize = length(scores);
      [ dummy, I ] = sort(scores);
      expectation = zeros(1, psize);
      expectation(I) = 1 ./ ((1:psize) .^ 0.5);
      expectation = pcount * expectation ./ sum(expectation);
    end

    function expectation = rankTop(ls, scores, pcount)
      psize = length(scores);
      quantity = round(0.25 * psize);
      [ dummy, I ] = sort(scores);
      expectation = zeros(1, psize);
      expectation(I(1:quantity)) = pcount / quantity;
    end

    function parents = select(ls, expectation, pcount, options)
      % High ranked individuals have a high probability to be selected.
      % +expected+ is the expected number of children for individuals.

      psize = length(expectation);

      % Normalize to have 1 in total
      expectation = expectation / sum(expectation);

      parents = zeros(1, pcount);
      for i = 1:pcount
        point = rand;
        com = 0;
        for j = 1:psize
          com = com + expectation(j);
          if point < com
            parents(i) = j;
            break;
          end
        end
      end
    end

    function children = crossover(ls, parents, options, genomeLength, ...
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

    function children = crossoverRealRandom(ls, parents, options, genomeLength, ...
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

    function children = mutate(ls, parents, options, chromosomeLength, ...
      fitnessFunc, state, thisScore, thisPopulation)

      ccount = length(parents);

      mprob = ls.tuning.mutationProbability;
      if ischar(mprob), mprob = eval(mprob); end

      children = zeros(ccount, chromosomeLength);

      for i = 1:ccount
        child = thisPopulation(parents(i), :);
        mutationPoints = find(rand(1, chromosomeLength) < mprob);
        child(mutationPoints) = ls.rand(1, length(mutationPoints));
        children(i, :) = child;
      end
    end

    function children = mutateNothing(ls, parents, options, chromosomeLength, ...
      fitnessFunc, state, thisScore, thisPopulation)

      children = parents;
    end

    function [ state, options, changed ] = output(ls, ...
      options, state, flag, interval)

      changed = false;

      if ls.drawing, ls.drawGeneration(state); end

      if state.Generation >= ls.tuning.generationLimit
        state.StopFlag = 'Exceed the number of generations';
        return;
      end

      state = ls.control(state);
    end

    function mobility = rand(ls, rows, cols)
      mobility = ls.minMobility + ...
        (ls.maxMobility - ls.minMobility) * rand(rows, cols);
    end

    function [ o, t ] = tune(ls, t)
      % Default options
      o = gaoptimset;

      % We know when to stop ourselves
      o.Generations = Inf;
      o.StallGenLimit = Inf;
      o.TolFun = 0;

      o.PopulationSize = t.populationSize;
      o.CrossoverFraction = t.crossoverFraction;

      o.FitnessScalingFcn = @ls.rank;
      o.SelectionFcn = @ls.select;
      o.CrossoverFcn = @ls.crossover;
      o.CreationFcn = @ls.create;
      o.MutationFcn = @ls.mutate;
      o.OutputFcns = [ @ls.output ];

      o.Display = 'off';
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
