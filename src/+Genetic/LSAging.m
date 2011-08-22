classdef LSAging < Genetic.LS
  properties (Access = protected)
    % Stop criteria
    generationalStall = 20; % generations
    generationalTolerance = 0.01; % percent of fitness
  end

  properties (Access = protected)
    lastBest
  end

  methods
    function ls = LSAging(varargin)
      ls = ls@Genetic.LS(varargin{:});
    end
  end

  methods (Access = protected)
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
          ls.leakageTolerance, ls.maxLeakageIterations);

      fitness = -min(Lifetime.predict(T));
    end

    function state = control(ls, state)
      if state.Generation < ls.generationalStall, return; end

      left = state.Best(end - ls.generationalStall + 1);
      right = state.Best(end);

      improvement = abs((right - left) / left);

      if improvement < ls.generationalTolerance
        state.StopFlag = 'Observed a generational stall';
      end
    end

    function initializeDrawing(ls)
      title('Aging');
      xlabel('Generation');
      ylabel('MTTF, time units');
      ls.lastBest = [];
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
