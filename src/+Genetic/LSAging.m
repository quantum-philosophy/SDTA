classdef LSAging < Genetic.LS
  properties (Access = protected)
    lastBest
  end

  methods (Static)
    function t = defaultTuning(varargin)
      t = Genetic.LS.defaultTuning( ...
        ... % Additional Stop criteria
        'generationStall', 20, ...
        'generationTolerance', 0.01, ...
        ... % Fraction of individuals who survive
        'generationalGap', 0.5, ...
        varargin{:} ...
      );
    end
  end

  methods
    function ls = LSAging(varargin)
      ls = ls@Genetic.LS(varargin{:});
    end
  end

  methods (Access = protected)
    function [ o, t ] = tune(ls, t)
      [ o, t ] = ls.tune@Genetic.LS(t);

      % These ones will survive for sure
      o.EliteCount = floor(t.generationalGap * t.populationSize);
    end

    function fitness = compute(ls, chromosome)
      % Make a new schedule
      LS.schedule(ls.graph, chromosome);

      if ls.graph.duration > ls.deadline
        fitness = 0;
        return;
      end

      % The graph is rescheduled now, obtain the dynamic power profile
      dynamicPowerProfile = Power.calculateDynamicProfile(ls.graph);

      % Get the temperature curve
      [ T, it, totalPowerProfile ] = ...
        ls.hotspot.solveCondensedEquationWithLeakage( ...
          dynamicPowerProfile, ls.vdd, ls.ngate, ...
          ls.tuning.leakageTolerance, ls.tuning.maxLeakageIterations);

      fitness = -min(Lifetime.predictMultiple(T));
    end

    function state = control(ls, state)
      if state.Generation < ls.tuning.generationStall, return; end

      left = state.Best(end - ls.tuning.generationStall + 1);
      right = state.Best(end);

      improvement = abs((right - left) / left);

      if improvement < ls.tuning.generationTolerance
        state.StopFlag = 'Observed a generational stall';
      end
    end

    function initializeDrawing(ls)
      axes(ls.drawing);
      title('Aging');
      xlabel('Generation');
      ylabel('MTTF, time units');
      ls.lastBest = [];
    end

    function drawGeneration(ls, state)
      scores = -state.Score;

      no = state.Generation;
      psize = length(scores);

      axes(ls.drawing);
      title([ 'Aging (generations ', num2str(state.Generation), ')' ]);

      score = max(scores);

      if ~isempty(ls.lastBest)
        line([ ls.lastBest(1), no ], [ ls.lastBest(2), score ], 'Color', 'b');
      end

      ls.lastBest = [ no, score ];

      line(ones(1, psize) * state.Generation, scores, ...
        'Line', 'None', 'Marker', 'x', 'Color', 'k');
    end
  end
end
