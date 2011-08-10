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

    % Power model
    K3 = 5.38e-7;
    K4 = 1.83;
    K5 = 4.19;
    Ij = 4.80e-10;
  end

  methods (Static)
    function profile = calculateDynamicProfile(graph)
      pes = graph.pes;
      mapping = graph.mapping;

      taskPower = zeros(1, length(mapping));

      for i = 1:length(pes)
        pe = pes{i};
        ids = find(mapping == i);
        if isempty(ids), continue; end
        types = graph.taskTypes(ids);

        taskPower(ids) = Power.calculateDynamic(pe.ceff(types), ...
          pe.frequency, pe.voltage);
      end

      profile = Power.distributePower(graph, taskPower);
    end

    function profile = calculateStaticProfile(pes, T)
      steps = size(T, 1);
      cores = size(T, 2);
      profile = zeros(steps, cores);

      for i = 1:cores
        profile(:, i) = Power.calculateStatic(...
          pes{i}.ngate, T(:, i), pes{i}.voltage);
      end
    end

    function profile = distributePower(graph, taskPower)
      startTime = graph.startTime;
      execTime = graph.execTime;
      mapping = graph.mapping;

      cores = length(graph.pes);

      finishTime = startTime + execTime;

      profile = zeros(0, cores);

      timeStep = Constants.samplingInterval;
      totalTime = max(finishTime);

      % ATTENTION: What should we do about this mismatch?
      steps = floor(totalTime / timeStep);
      if steps * timeStep < totalTime, steps = steps + 1; end

      for i = 1:cores
        % Find all tasks for this core
        ids = find(mapping == i);
        tasks = length(ids);

        if tasks == 0, continue; end

        % Sort them according to their start times
        [ dummy, I ] = sort(startTime(ids));
        ids = ids(I);

        for id = ids
          s = floor(startTime(id) / timeStep) + 1;
          % NOTE: Here without +1 to eliminate successor and predecessor
          % are being running at the same time
          e = floor(finishTime(id) / timeStep);
          profile(s:e, i) = taskPower(id);
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

      % f = 1: Vdd ~= 4.03
    end

    function Is = calculateIs(Vdd, Vbs)
      % The leakage power for just one gate
      %
      % "Combined Dynamic Voltage Scaling and Adaptive Body Biasing for
      % Lower Power Microprocessors under Dynamic Workloads"
      %
      % This model is for the 65nm technology, and it seems that it is
      % only valid for the temperature equal to 27C = 300.15K
      %
      Pleak = Vdd * Power.K3 * exp(Power.K4 * Vdd) * exp(Power.K5 * Vbs) + ...
        abs(Vbs) * Power.Ij;

      % Pleak = Is * Vdd
      % Is = Iavg(Tref, Vddref)
      %
      % "Temperature and Supply Voltage Aware Performance and
      % Power Modeling at Microarchitecture Level"
      %
      Is = Pleak / Vdd;
    end

    function Is = calculateMeanIs()
      T = [ 100, 100, 80, 80, 60, 60 ] + Constants.degreeKelvin;
      V = [ 0.95, 1.05, 0.95, 1.05, 0.95, 1.05 ];
      Iavg = [ 23.44, 29.56, 19.44, 25.14, 16.00, 21.33 ] * 1e-6;
      Is = mean(Iavg ./ Power.calculateScaling(T, V));
    end
  end
end
