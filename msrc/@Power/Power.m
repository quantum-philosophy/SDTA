classdef Power < handle
  properties (Constant)
    % Scaling coefficients of the average leakage current for 65nm
    A = 1.1432e-12;
    B = 1.0126e-14;
    alpha = 466.4029;
    beta = -1224.74083;
    gamma = 6.28153;
    delta = 6.9094;
    Is = Power.calculateMeanIs();
  end

  methods (Static)
    function profile = fitProfile(powerProfile, steps)
      currentSteps = size(powerProfile, 1);

      if steps < currentSteps
        profile = powerProfile(1:steps, :);
      elseif steps > currentSteps
        repeat = floor(steps / currentSteps);
        profile = zeros(0, 0);
        for i = 1:repeat
          profile = [ profile; powerProfile ];
        end
        rest = steps - repeat * currentSteps;
        profile = [ profile; powerProfile(1:rest, :) ];
      end
    end

    function profile = generateConstantProfile(cores, steps, maxPower)
      profile = ones(steps, cores) * (maxPower / cores);
    end

    function profile = generateRandomProfile(cores, steps, maxPower)
      profile = 0.2 + rand(steps, cores);
      for i = 1:steps
        multiplier = maxPower / sum(profile(i, :));
        profile(i, :) = multiplier * profile(i, :);
      end
    end

    function profile = calculateDynamicProfile(graph, varargin)
      taskPower = Power.calculateTask(graph);
      profile = Power.distributePower(graph, taskPower, varargin{:});
    end

    function taskPower = calculateTask(graph)
      taskPower = zeros(1, length(graph.tasks));

      for pe = graph.pes
        pe = pe{1};

        for task = graph.getPETasks(pe)
          task = task{1};
          taskPower(task.id) = Power.calculateDynamic(...
            pe.ceff(task.type), pe.frequency, pe.voltage);
        end
      end
    end

    function profile = calculateStaticProfile(pes, T)
      stepCount = size(T, 1);
      peCount = size(T, 2);
      profile = zeros(stepCount, peCount);

      for i = 1:peCount
        profile(:, i) = Power.calculateStatic(...
          pes{i}.ngate, T(:, i), pes{i}.voltage);
      end
    end

    function profile = distributePower(graph, taskPower, samplingInterval)
      if nargin < 3
        ts = Constants.samplingInterval;
      else
        ts = samplingInterval;
      end

      % Here we build a profile for the whole time period of the graph
      % including its actual duration (only tasks) plus the gap to
      % the deadline
      profile = zeros(ceil(graph.deadline / ts), length(graph.pes));

      for pe = graph.pes
        pe = pe{1};

        schedule = graph.getPESchedule(pe);

        for id = schedule
          task = graph.tasks{id};
          s = floor(task.start / ts) + 1;
          % NOTE: Here without +1 to eliminate successor and predecessor
          % are being running at the same time
          e = floor((task.start + task.duration) / ts);
          profile(s:e, pe.id) = taskPower(id);
        end
      end
    end

    function Pdyn = calculateDynamic(Ceff, f, Vdd)
      Pdyn = Ceff .* f .* Vdd.^2;
    end

    function Pleak = calculateStatic(Ngate, T, Vdd)
      % Pleak = Ngate * Iavg * Vdd
      % Iavg(T, Vdd) = Is(T0, V0) * favg(T, Vdd)
      %
      % "Temperature and Supply Voltage Aware Performance and
      % Power Modeling at Microarchitecture Level"
      %
      % The average leakage current per gate
      Iavg = Power.Is * Power.calculateScaling(T, Vdd);

      % The power leakage for all gates
      Pleak = Ngate .* Iavg .* Vdd;
    end

    function f = calculateScaling(T, Vdd)
      % The scaling factor
      %
      % f(T, Vdd) = A * T^2 * e^((alpha * Vdd + beta)/T) +
      %   B * e^(gamma * Vdd + delta)
      %
      % "Temperature and Supply Voltage Aware Performance and
      % Power Modeling at Microarchitecture Level"
      %
      f = Power.A .* T.^2 .* exp((Power.alpha .* Vdd + Power.beta) ./ T) + ...
        Power.B .* exp(Power.gamma .* Vdd + Power.delta);

      % f = 1: Vdd ~= 4.03, the authors say it is not true
    end

    function Is = calculateMeanIs()
      T = [ 100, 100, 80, 80, 60, 60 ] + Constants.degreeKelvin;
      V = [ 0.95, 1.05, 0.95, 1.05, 0.95, 1.05 ];
      Iavg = [ 23.44, 29.56, 19.44, 25.14, 16.00, 21.33 ] * 1e-6;
      Is = mean(Iavg ./ Power.calculateScaling(T, V));
    end
  end
end
