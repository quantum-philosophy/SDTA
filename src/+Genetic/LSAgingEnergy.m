classdef LSAgingEnergy < Genetic.LS
  properties (Access = private)
    currentLine
  end

  methods
    function ls = LSAgingEnergy(varargin)
      ls = ls@Genetic.LS(varargin{:});
      ls.options.FitnessScalingFcn = [];
      ls.options.SelectionFcn = [];
    end

    function [ solution, fitness, output ] = solve(ls, drawing)
      if nargin > 0, ls.initializeDrawing(drawing); end

      % Reset
      ls.cache = containers.Map('KeyType', 'char', 'ValueType', 'any');
      ls.evaluations = 0;

      ls.bar = waitbar(0, 'Genetic List Scheduling Algorithm');

      [ solution, fitness, exitflag, output ] = gamultiobj(@ls.evaluate, ...
          ls.chromosomeLength, [], [], [], [], [], [], ls.options);

      delete(ls.bar);
    end
  end

  methods (Access = protected)
    function fitness = evaluate(ls, chromosome)
      ls.evaluations = ls.evaluations + 1;
      waitbar(mod(ls.evaluations, 10) / 10, ls.bar, ...
        [ 'Evaluation #' num2str(ls.evaluations) ]);

      key = Utils.mMD5(chromosome);

      if ls.cache.isKey(key)
        fitness = ls.cache(key);
        return;
      end

      % Make a new schedule
      LS.schedule(ls.graph, chromosome);

      if ls.graph.duration > ls.deadline
        % Respect the deadline!
        fitness = [ Inf, Inf ];
      else
        % The graph is rescheduled now, obtain the dynamic power profile
        dynamicPowerProfile = Power.calculateDynamicProfile(ls.graph);

        % Get the temperature curve
        [ T, it, totalPowerProfile ] = ...
          ls.hotspot.solveCondensedEquationWithLeakage( ...
            dynamicPowerProfile, ls.vdd, ls.ngate, ...
            ls.leakageTolerance, ls.maxLeakageIterations);

        % We want to prolong aging
        aging = -min(Lifetime.predict(T));
        % ... and we want to keep the energy low
        energy = sum(sum(totalPowerProfile)) * Constants.samplingInterval;

        fitness = [ aging, energy ];
      end

      % Cache it!
      ls.cache(key) = fitness;
    end

    function [ state, options, changed ] = control(ls, ...
      options, state, flag, interval)

      changed = false;

      if ls.drawing, ls.drawGeneration(state); end

      no = state.Generation;

      if no >= ls.generationLimit
        state.StopFlag = 'Exceed the number of generations';
      end
    end

    function initializeDrawing(ls, drawing)
      ls.drawing = drawing;
      ls.currentLine = [];
      title('Energy and Aging');
      ylabel('Energy consumption, J');
      xlabel('Lifetime duration, time units');
      grid on;
    end

    function drawGeneration(ls, state)
      if state.Generation == 0, return; end

      index = find(state.Rank == 1);
      aging = -state.Score(index, 1);
      energy = state.Score(index, 2);

      figure(ls.drawing);
      title([ 'Energy and Aging (generation ', num2str(state.Generation), ')' ]);

      if ~isempty(ls.currentLine)
        set(ls.currentLine, ...
          'Color', 'k', 'Line', 'none', 'Marker', 'x');
      end

      ls.currentLine = line(aging, energy, ...
        'Color', 'r', 'Line', 'none', 'Marker', 'o');
    end
  end
end
