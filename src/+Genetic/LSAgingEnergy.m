classdef LSAgingEnergy < Genetic.LS
  properties (Access = private)
    currentLine
  end

  methods
    function ls = LSAgingEnergy(varargin)
      ls = ls@Genetic.LS(varargin{:});

      % Another solver
      ls.solver = @gamultiobj;
      ls.additionalParams = cell(1, 6);

      % Caching
      ls.fitnessType = 'any';
    end
  end

  methods (Access = protected)
    function [ o, t ] = tune(ls, t)
      [ o, t ] = ls.tune@Genetic.LS(t);

      % Multi-objective version does not have this
      o.FitnessScalingFcn = [];
      o.SelectionFcn = [];
    end

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
          ls.tuning.leakageTolerance, ls.tuning.maxLeakageIterations);

      % We want to prolong aging
      aging = Lifetime.predict(T);
      % ... and we want to keep the energy low
      energy = sum(sum(totalPowerProfile)) * Constants.samplingInterval;

      fitness = [ -aging, energy ];
    end

    function state = control(ls, state)
      % Without any special control
    end

    function initializeDrawing(ls)
      axes(ls.drawing);
      title('Aging vs Energy');
      ylabel('Energy, J');
      xlabel('MTTF, time units');
      ls.currentLine = [];
    end

    function drawGeneration(ls, state)
      if state.Generation == 0, return; end

      % index = find(state.Rank == 1);
      aging = -state.Score(:, 1);
      energy = state.Score(:, 2);

      axes(ls.drawing);
      title([ 'Aging vs Energy (generations ', num2str(state.Generation), ')' ]);

      if ~isempty(ls.currentLine)
        set(ls.currentLine, ...
          'Color', 'k', 'Line', 'none', 'Marker', 'x');
      end

      ls.currentLine = line(aging, energy, ...
        'Color', 'r', 'Line', 'none', 'Marker', 'o');
    end
  end
end
