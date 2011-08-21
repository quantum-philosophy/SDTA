classdef LSAging < Genetic.LS
  properties (Access = private)
    lastBest
  end

  methods
    function ls = LSAging(varargin)
      ls = ls@Genetic.LS(varargin{:});
    end

    function [ solution, fitness, output ] = solve(ls, drawing)
      if nargin > 0, ls.initializeDrawing(drawing); end

      % Reset
      ls.cache = containers.Map('KeyType', 'char', 'ValueType', 'double');
      ls.evaluations = 0;

      ls.bar = waitbar(0, 'Genetic List Scheduling Algorithm');

      [ solution, fitness, exitflag, output ] = ga(@ls.evaluate, ...
        ls.chromosomeLength, [], [], [], [], [], [], [], ls.options);

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
        fitness = 0;
      else
        % The graph is rescheduled now, obtain the dynamic power profile
        dynamicPowerProfile = Power.calculateDynamicProfile(ls.graph);

        % Get the temperature curve
        [ T, it, totalPowerProfile ] = ...
          ls.hotspot.solveCondensedEquationWithLeakage( ...
            dynamicPowerProfile, ls.vdd, ls.ngate, ...
            ls.leakageTolerance, ls.maxLeakageIterations);

        fitness = -min(Lifetime.predict(T));
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
      elseif no >= ls.generationalStall
        left = state.Best(end - ls.generationalStall + 1);
        right = state.Best(end);

        improvement = abs((right - left) / left);

        if improvement < ls.generationalTolerance
          state.StopFlag = 'Observed a generational stall';
        end
      end
    end

    function initializeDrawing(ls, drawing)
      ls.drawing = drawing;
      title('Lifetime');
      xlabel('Generation');
      ylabel('Lifetime, time units');
      grid on;
    end

    function drawGeneration(ls, state)
      scores = -state.Score;

      no = state.Generation;
      psize = length(scores);

      figure(ls.drawing);

      currentBest = max(scores);

      if ~isempty(ls.lastBest)
        line([ no - 1, no ], [ ls.lastBest, currentBest ], 'Color', 'b');
      end

      ls.lastBest = currentBest;

      line(ones(1, psize) * state.Generation, scores, ...
        'Line', 'None', 'Marker', 'x', 'Color', 'r');
    end
  end
end
