classdef LS < handle
  properties (Constant)
    % Maximal number of iterations for the leakage loop
    maxLeakageIterations = 10;

    % Tolerance for the leakage loop
    leakageTolerance = 0.01; % K
  end

  properties (SetAccess = protected)
    % Stop criteria
    generationLimit = 200;
    generationalStall = 20; % generations
    generationalTolerance = 0.01; % percent of fitness

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
  end

  properties (SetAccess = protected)
    options

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
  end

  methods (Abstract)
    [ solution, fitness, output ] = solve()
  end

  methods (Abstract, Access = protected)
    [ state, options, changed ] = control(options, state, flag, interval)
  end

  methods
    function ls = LS(graph, hotspot)
      % Configure GA
      options = gaoptimset;

      % We know when to stop ourselves
      options.Generations = Inf;
      options.StallGenLimit = Inf;
      options.TolFun = 0;

      options.PopulationSize = ls.populationSize;
      options.EliteCount = floor(ls.generationalGap * ls.populationSize);
      options.CrossoverFraction = ls.crossoverFraction;

      options.FitnessScalingFcn = @ls.rank;
      options.SelectionFcn = @ls.select;
      options.CrossoverFcn = @ls.crossover;
      options.CreationFcn = @ls.create;
      options.MutationFcn = @ls.mutate;
      options.OutputFcns = [ @ls.control ];

      ls.options = options;

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
    end
  end

  methods (Access = protected)
    function population = create(ls, chromosomeLength, fitnessFcn, options)
      psize = ls.populationSize;

      population = zeros(psize, chromosomeLength);
      half = floor(ls.mobilityCreationFactor * psize);

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

      % To mutate or not to mutate? That is the question...
      % The probability to mutate should not be less than 15%
      mprob = max(ls.minimalMutationProbability, ...
        1 / exp(state.Generation * 0.05));

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

    function mobility = rand(ls, rows, cols)
      mobility = ls.minMobility + ...
        (ls.maxMobility - ls.minMobility) * rand(rows, cols);
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
