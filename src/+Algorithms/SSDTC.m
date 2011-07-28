classdef SSDTC < handle
  properties (Access = private)
    % Tasks
    graph

    % PEs
    voltage
    frequency
    ceff
    nc

    mapping
  end

  properties (SetAccess = private)
    coreCount
    taskCount
    stepCount

    thermalModel
    powerProfile
  end

  methods
    function ssdtc = SSDTC(graph, pes, comms, floorplan, config)
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
      % scheduler = Algorithms.GLSA();
      % [ solution, fitness, flag ] = ...
      %   scheduler.process(graph, @ssdtc.evaluateSchedule);

      scheduler = Algorithms.LS(graph);
      ssdtc.calculatePowerProfile(scheduler.schedule);

      ssdtc.thermalModel = Algorithms.TM(floorplan, config);
    end

    function inspect(ssdtc)
      for graph = ssdtc.graphs, graph{1}.inspect(); end
    end

    function T = solveWithCondensedEquation(ssdtc)
      T = ssdtc.thermalModel.solveWithCondensedEquation(ssdtc.powerProfile);
    end

    function T = solveWithHotSpot(ssdtc)
      T = ssdtc.thermalModel.solveWithHotSpot(ssdtc.powerProfile);
    end

    function dumpPowerProfile(ssdtc, file)
      Utils.dumpPowerProfile(file, ssdtc.powerProfile);
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
      timeStep = Algorithms.TM.samplingInterval;

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
      ssdtc.powerProfile = powerProfile;
    end
  end
end
