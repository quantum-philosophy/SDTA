classdef SSDTC < handle
  properties (Constant)
    samplingInterval = 0.002;
  end

  properties (Access = private)
    % Tasks
    graph
    taskCount

    % PEs
    peCount
    voltage
    frequency
    ceff
    nc

    hotspotConfig
    floorplan
    mapping
  end

  methods
    function ssdtc = SSDTC(varargin)
      if nargin > 0, ssdtc.process(varargin{:}); end
    end

    function process(ssdtc, graph, pes, comms, hotspotConfig, floorplan)
      if length(pes) ~= 1
        fprintf('Exactly one core is supported right now\n');
        return;
      end

      % Tasks
      ssdtc.graph = graph;
      ssdtc.taskCount = length(graph.tasks);

      % PEs
      ssdtc.peCount = 0;
      ssdtc.voltage = zeros(0, 0);
      ssdtc.frequency = zeros(0, 0);
      ssdtc.ceff = zeros(0, 0);
      ssdtc.nc = zeros(0, 0);
      for pe = pes, ssdtc.addPE(pe{1}); end

      ssdtc.hotspotConfig = hotspotConfig;
      ssdtc.floorplan = floorplan;

      % TODO: Communications

      % TODO: Mapping, dummy for the moment
      ssdtc.mapping = randi(ssdtc.peCount, 1, ssdtc.taskCount);

      % TODO: Genetic list scheduler algorithm
      % scheduler = GLSA();
      % [ solution, fitness, flag ] = ...
      %   scheduler.process(graph, @ssdtc.evaluateSchedule);

      ls = LS(graph);
      powerProfile = ssdtc.calculatePowerProfile(ls.schedule);
    end

    function inspect(ssdtc)
      for graph = ssdtc.graphs, graph{1}.inspect(); end
    end
  end

  methods (Access = private)
    function addPE(ssdtc, pe)
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
      ssdtc.peCount = ssdtc.peCount + 1;
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

    function powerProfile = calculatePowerProfile(ssdtc, schedule)
      cores = ssdtc.peCount;
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
    end
  end
end
