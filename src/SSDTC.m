classdef SSDTC < handle
  properties (Constant)
    samplingInterval = 2e-3;    % Sampling interval
    ambientTemperature = 45.0;  % Ambient temperature
  end

  properties (Access = private)
    % Tasks
    graph

    % PEs
    voltage
    frequency
    ceff
    nc

    hotspotConfig
    floorplan

    mapping

    D           % sqrt(invC) * A * sqrt(invC)
    DL          % Eigenvalues of D
    DV          % Eigenvectors of D
    DVT         % Transposed eigenvectors of D
    sinvC       % Square root from the inverse of C
    B           % Power profile
  end

  properties (SetAccess = private)
    temperatureCurve

    coreCount
    taskCount
    nodeCount
    stepCount
  end

  methods
    function ssdtc = SSDTC(varargin)
      if nargin > 0, ssdtc.process(varargin{:}); end
    end

    function T = process(ssdtc, graph, pes, comms, floorplan, config)
      if length(pes) ~= 1
        fprintf('Exactly one core is supported right now\n');
        return;
      end

      % Tasks
      ssdtc.graph = graph;
      ssdtc.taskCount = length(graph.tasks);

      % PEs
      ssdtc.coreCount = 0;
      ssdtc.voltage = zeros(0, 0);
      ssdtc.frequency = zeros(0, 0);
      ssdtc.ceff = zeros(0, 0);
      ssdtc.nc = zeros(0, 0);
      for pe = pes, ssdtc.addProcessingElement(pe{1}); end

      % TODO: Communications

      % TODO: Mapping, dummy for the moment
      ssdtc.mapping = randi(ssdtc.coreCount, 1, ssdtc.taskCount);

      % TODO: Genetic list scheduler algorithm
      % scheduler = GLSA();
      % [ solution, fitness, flag ] = ...
      %   scheduler.process(graph, @ssdtc.evaluateSchedule);

      ls = LS(graph);

      ssdtc.calculateThermalModel(floorplan, config);
      ssdtc.calculatePowerProfile(ls.schedule);
      ssdtc.calculateTemperatureCurve();
    end

    function inspect(ssdtc)
      for graph = ssdtc.graphs, graph{1}.inspect(); end
    end
  end

  methods (Access = private)
    function addProcessingElement(ssdtc, pe)
      % Validate PE attributes
      if ~pe.attributes.isKey('frequency')
        fprintf('Can not determine frequency for %s %d\n', pe.name, pe.id);
        return;
      end
      if ~pe.attributes.isKey('voltage')
        fprintf('Can not determine voltage for %s %d\n', pe.name, pe.id);
        return;
      end

      % Validate type attributes
      ceff = [];
      nc = [];
      for i = 1:length(pe.header)
        if strcmp(pe.header{i}, 'effective_switched_capacitance')
          ceff = pe.values(:, i);
        elseif strcmp(pe.header{i}, 'number_of_clock_cycles')
          nc = pe.values(:, i);
        end
      end

      % Effective switched capacitance
      if isempty(ceff)
        fprintf('Can not determine Ceff for %s %d\n', pe.name, pe.id);
        return;
      end

      % Number of clock cycles
      if isempty(nc)
        fprintf('Can not determine NC for %s %d\n', pe.name, pe.id);
        return;
      end

      % Everything is fine, write!
      ssdtc.coreCount = ssdtc.coreCount + 1;
      ssdtc.voltage(end + 1) = pe.attributes('voltage');
      ssdtc.frequency(end + 1) = pe.attributes('frequency');
      ssdtc.ceff(end + 1, 1:length(ceff)) = ceff;
      ssdtc.nc(end + 1, 1:length(nc)) = nc;
    end

    function fitness = evaluateSchedule(ssdtc, schedule)
      fitness = ssdtc.estimateEnergy(schedule);
    end

    function energy = estimateEnergy(ssdtc, schedule);
      energy = 0;
    end

    function calculatePowerProfile(ssdtc, schedule)
      cores = ssdtc.coreCount;
      tasks = ssdtc.taskCount;

      taskTime = zeros(cores, tasks);
      taskPower = zeros(cores, tasks);

      for i = 1:cores
        fprintf('PE %d\n', 1);
        fprintf('  frequency = %f GHz\n', ssdtc.frequency(i) * 10^(-9));
        fprintf('  voltage = %f\n', ssdtc.voltage(i));

        taskIds = find(ssdtc.mapping == i);
        taskTypes = ssdtc.graph.taskTypes(taskIds);

        % t = NC / f
        taskTime(i, taskIds) = ssdtc.nc(i, taskTypes) / ssdtc.frequency(i);

        % Pdyn = Ceff * f * Vdd^2
        taskPower(i, taskIds) = ssdtc.ceff(i, taskTypes) * ...
          ssdtc.frequency(i) * ssdtc.voltage(i)^2;
      end

      powerProfile = zeros(0, cores);
      timeStep = ssdtc.samplingInterval;

      totalTime = sum(taskTime); % seconds
      fprintf('total time = %f s\n', totalTime);

      steps = floor(totalTime / timeStep);
      if steps * timeStep < totalTime, steps = steps + 1; end
      fprintf('sampling interval = %f s\n', timeStep);
      fprintf('power steps = %d\n', steps);
      fprintf('time mismatch = %f\n', steps * timeStep - totalTime);

      for i = 1:cores
        k = 1;
        futureTime = taskTime(i, schedule(k)); % Time in tasks
        currentTime = 0; % Time of the current step
        for j = 1:steps
          while currentTime >= futureTime
            k = k + 1;
            futureTime = futureTime + taskTime(i, schedule(k));
          end
          powerProfile(j, i) = taskPower(i, schedule(k));
          currentTime = currentTime + timeStep;
        end
      end

      ssdtc.stepCount = steps;
      ssdtc.B = powerProfile;
    end

    function calculateThermalModel(ssdtc, floorplan, config)
      [ negA, invC ] = obtainHotSpotModel(floorplan, config);
      invC = diag(diag(invC));

      sinvC = sqrt(invC);
      D = symmetrize(sinvC * (- negA) * sinvC);

      [ V, L ] = eig(D);

      ssdtc.D = D;
      ssdtc.DL = diag(L);
      ssdtc.DV = V;
      ssdtc.DVT = V';
      ssdtc.sinvC = sinvC;
      ssdtc.nodeCount = length(D);
    end

    function calculateTemperatureCurve(ssdtc)
      n = ssdtc.nodeCount;
      m = ssdtc.stepCount;
      cores = ssdtc.coreCount;
      nm = n * m;

      sinvC = ssdtc.sinvC;
      DL = ssdtc.DL;
      DV = ssdtc.DV;
      DVT = ssdtc.DVT;
      ts = ssdtc.samplingInterval;
      at = ssdtc.ambientTemperature;

      B = transpose(ssdtc.B);
      B = [ B; zeros(n - cores, m) ];

      % exp(D * t) = U * diag(exp(li * t)) * UT
      %
      K = DV * diag(exp(ts * DL)) * DVT;

      % G = D^(-1) (exp(D * t) - I) C^(-1/2) =
      % U * diag((exp(li * t) - 1) / li) * UT * C^(-1/2)
      %
      G = DV * diag((exp(ts * DL) - 1) ./ DL) * DVT * sinvC;

      P = zeros(n, m);
      Q = zeros(n, m);
      Q(:, 1) = G * B(:, 1);
      P(:, 1) = Q(:, 1);
      for i = 2:m
        Q(:, i) = G * B(:, i);
        P(:, i) = K * P(:, i - 1) + Q(:, i);
      end

      Y = zeros(nm, 1);
      Y(1:n) = DV * diag(1 ./ (1 - exp(ts * m * DL))) * DVT * P(:, m);

      for i = 2:m
        op = (i - 2) * n + 1;
        on = op + n;
        Y(on:(on + n - 1)) = K * Y(op:(op + n - 1)) + Q(:, i - 1);
      end

      T = zeros(n, m);
      T(:, 1) = DV * diag(1 ./ (1 - exp(ts * m * DL))) * DVT * P(:, m);

      for i = 2:m
        T(:, i) = K * T(:, i - 1) + Q(:, i - 1);
      end

      T = transpose(T);
      dsinvC = transpose(diag(sinvC));

      for i = 1:m
        T(i, :) = T(i, :) .* dsinvC + at;
      end

      ssdtc.temperatureCurve = T;
    end
  end
end
