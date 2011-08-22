classdef LSAgingEnergy < Genetic.LS
  properties (Access = private)
    currentLine
  end

  methods
    function ls = LSAgingEnergy(varargin)
      ls = ls@Genetic.LS(varargin{:});

      % Solver itself
      ls.solver = @gamultiobj;
      ls.additionalParams = cell(1, 6);

      % Multiobjective version does not have this
      ls.options.FitnessScalingFcn = [];
      ls.options.SelectionFcn = [];

      % Caching
      ls.fitnessType = 'any';

      % Tuning
      ls.generationLimit = 50;
      ls.populationSize = 40;
    end
  end

  methods (Access = protected)
    function fitness = compute(ls, chromosome)
      % Make a new schedule
      LS.schedule(ls.graph, chromosome);

      if ls.graph.duration > ls.deadline
        % Respect the deadline!
        fitness = [ Inf, Inf ];
        return;
      end

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

    function state = control(ls, state)
      % Without any special control
    end

    function initializeDrawing(ls)
      title('Aging vs Energy');
      ylabel('Energy, J');
      xlabel('MTTF, time units');
      ls.currentLine = [];
    end

    function drawGeneration(ls, state)
      if state.Generation == 0, return; end

      index = find(state.Rank == 1);
      aging = -state.Score(index, 1);
      energy = state.Score(index, 2);

      figure(ls.drawing);
      title([ 'Aging vs Energy (generation ', num2str(state.Generation), ')' ]);

      if ~isempty(ls.currentLine)
        set(ls.currentLine, ...
          'Color', 'k', 'Line', 'none', 'Marker', 'x');
      end

      ls.currentLine = line(aging, energy, ...
        'Color', 'r', 'Line', 'none', 'Marker', 'o');
    end
  end
end
