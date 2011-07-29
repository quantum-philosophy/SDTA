classdef SSDTC < handle
  properties (Access = private)
    % Tasks
    graph

    % PEs
    pes
    voltage
    frequency
    ceff
    nc

    thermalModel
    mapping
    schedule
    execTime
    startTime
    powerProfile
    temperatureProfile
  end

  properties (SetAccess = private)
    coreCount
    taskCount
    stepCount
  end

  methods
    function ssdtc = SSDTC(graph, pes, floorplan, config)
      % Tasks
      ssdtc.graph = graph;
      ssdtc.taskCount = length(graph.tasks);

      % PEs
      ssdtc.pes = {};
      ssdtc.coreCount = 0;
      ssdtc.voltage = zeros(0, 0);
      ssdtc.frequency = zeros(0, 0);
      ssdtc.ceff = zeros(0, 0);
      ssdtc.nc = zeros(0, 0);
      for pe = pes, ssdtc.addProcessingElement(pe{1}); end

      % Thermal model
      ssdtc.thermalModel = Algorithms.TM(floorplan, config);

      % Dummy mapping
      ssdtc.mapping = randi(ssdtc.coreCount, 1, ssdtc.taskCount);

      % LS scheduling
      Utils.startTimer('List scheduling');
      ssdtc.schedule = Algorithms.LS(graph);
      Utils.stopTimer();

      % Power profile
      Utils.startTimer('Generate a power profile');
      [ ssdtc.powerProfile, ssdtc.startTime, ssdtc.execTime ] = ...
        ssdtc.calculatePowerProfile(ssdtc.mapping, ssdtc.schedule);
      Utils.stopTimer();

      ssdtc.stepCount = size(ssdtc.powerProfile, 1);
    end

    function inspect(ssdtc, draw)
      if nargin < 2, draw = false; end

      % Graph
      ssdtc.graph.inspect();

      % PEs
      for pe = ssdtc.pes, pe{1}.inspect(); end

      % Mapping
      Utils.inspectVector('Mapping', ssdtc.mapping);

      % Schedule
      Utils.inspectVector('Schedule', ssdtc.schedule);

      % Power
      fprintf('steps = %d\n', ssdtc.stepCount);
      fprintf('sampling interval = %f s\n', Algorithms.TM.samplingInterval);
      fprintf('total time = %f s\n', Algorithms.TM.samplingInterval * ssdtc.stepCount);

      if draw
        ssdtc.drawSimulation(ssdtc.mapping, ssdtc.startTime, ssdtc.execTime, ...
          ssdtc.powerProfile, ssdtc.temperatureProfile);
      end
    end

    function T = solveWithCondensedEquation(ssdtc)
      Utils.startTimer('Solve with condensed equation');
      T = ssdtc.thermalModel.solveWithCondensedEquation(ssdtc.powerProfile);
      Utils.stopTimer();
      ssdtc.temperatureProfile = T;
    end

    function T = solveWithHotSpot(ssdtc, varargin)
      Utils.startTimer('Solve with HotSpot');
      T = ssdtc.thermalModel.solveWithHotSpot(ssdtc.powerProfile, varargin{:});
      Utils.stopTimer();
      ssdtc.temperatureProfile = T;
    end

    function dumpPowerProfile(ssdtc, file)
      Utils.dumpPowerProfile(file, ssdtc.powerProfile);
    end
  end

  methods (Access = private)
    function addProcessingElement(ssdtc, pe)
      % Validate PE attributes
      if ~pe.attributes.isKey('frequency')
        Utils.eprintf('Can not determine frequency for %s %d\n', pe.name, pe.id);
        return;
      end
      if ~pe.attributes.isKey('voltage')
        Utils.eprintf('Can not determine voltage for %s %d\n', pe.name, pe.id);
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
        Utils.eprintf('Can not determine Ceff for %s %d\n', pe.name, pe.id);
        return;
      end

      % Number of clock cycles
      if isempty(nc)
        Utils.eprintf('Can not determine NC for %s %d\n', pe.name, pe.id);
        return;
      end

      % Everything is fine, write!
      ssdtc.pes{end + 1} = pe;
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

    function [ powerProfile, startTime, execTime ] = calculatePowerProfile(...
      ssdtc, mapping, schedule)

      cores = ssdtc.coreCount;

      [ startTime, execTime, taskPower ] = ...
        ssdtc.calculateTaskConstants(mapping, schedule);

      finishTime = startTime + execTime;

      powerProfile = zeros(0, cores);

      timeStep = Algorithms.TM.samplingInterval;
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
          powerProfile(s:e, i) = taskPower(id);
        end
      end
    end

    function [ startTime, execTime, taskPower ] = calculateTaskConstants(ssdtc, mapping, schedule)
      cores = ssdtc.coreCount;
      tasks = ssdtc.taskCount;

      execTime = zeros(1, tasks);
      startTime = zeros(1, tasks);
      taskPower = zeros(1, tasks);

      coreSchedule = cell(cores);

      for i = 1:cores
        coreSchedule{i} = zeros(0, 0);

        ids = find(mapping == i);
        if isempty(ids), continue; end

        types = ssdtc.graph.taskTypes(ids);

        % t = NC / f
        execTime(ids) = ssdtc.nc(i, types) / ssdtc.frequency(i);

        % Pdyn = Ceff * f * Vdd^2
        taskPower(ids) = ssdtc.ceff(i, types) * ...
          ssdtc.frequency(i) * ssdtc.voltage(i)^2;

        % Calculate a local schedule, shift its tasks relative to each other
        shift = 0;
        for id = schedule
          if any(ids == id)
            coreSchedule{i}(end + 1) = id;
            startTime(id) = shift;
            shift = shift + execTime(id);
          end
        end
      end

      % Now consider dependencies between tasks
      pool = schedule;
      inpool = ones(1, tasks);
      while ~isempty(pool)
        id = pool(1);
        pool(1) = [];
        inpool(id) = 0;

        finish = startTime(id) + execTime(id);

        nids = ssdtc.graph.taskIndexesFrom{id};
        for nid = nids
          shift = finish - startTime(nid);
          if shift < 0, continue; end

          % Shift the core schedule
          ncid = mapping(nid);
          found = 0;
          for sid = coreSchedule{ncid}
            if ~found && sid == nid, found = 1; end
            if found
              startTime(sid) = startTime(sid) + shift;
              if ~inpool(sid)
                % We need to consider it once again
                pool(end + 1) = sid;
                inpool(sid) = 1;
              end
            end
          end
        end
      end
    end

    function drawSimulation(ssdtc, mapping, startTime, execTime,...
      powerProfile, temperatureProfile)

      cores = ssdtc.coreCount;
      tasks = ssdtc.taskCount;

      colors = { 'r', 'g', 'b', 'm', 'y', 'c' };

      plots = 1;
      if nargin < 5 || isempty(powerProfile)
        powerProfile = [];
      else
        plots = plots + 1;
      end
      if nargin < 6 || isempty(temperatureProfile)
        temperatureProfile = [];
      else
        plots = plots + 1;
      end

      % Mapping and scheduling
      subplot(plots, 1, 1);
      title('Mapping and Scheduling');
      xlabel('Time, s');
      ylabel('Cores');

      height = 0.5;
      for i = 1:cores
        t = 0;
        ids = find(mapping == i);
        [ d, I ] = sort(startTime(ids));
        x = [ 0 ];
        y = [ i ];
        for id = ids(I)
          x(end + 1) = startTime(id);
          y(end + 1) = i;

          x(end + 1) = startTime(id);
          y(end + 1) = i + height;

          x(end + 1) = startTime(id) + execTime(id);
          y(end + 1) = i + height;

          x(end + 1) = startTime(id) + execTime(id);
          y(end + 1) = i;

          text(startTime(id), i + 0.5 * height, sprintf('  %d', id));
        end
        color = colors{mod(i - 1, length(colors)) + 1};
        line(x, y, 'Color', color);
      end

      set(gca, 'YTick', 1:cores);

      if ~isempty(powerProfile)
        % Power profile
        subplot(plots, 1, 2);
        title('Power Profile');
        xlabel('Time, s');
        ylabel('Power, W');

        x = 1:size(powerProfile, 1);
        x = (x - 1) * Algorithms.TM.samplingInterval;
        for i = 1:cores
          color = colors{mod(i - 1, length(colors)) + 1};
          line(x, powerProfile(:, i), 'Color', color);
        end

        % Overall power consumption
        line(x, sum(powerProfile, 2), 'Color', 'k', 'Line', '--');
      end

      if ~isempty(temperatureProfile)
        % Temperature
        subplot(plots, 1, 3);
        title('Temperature Profile');
        xlabel('Time, s');
        ylabel('Temperature, C');

        x = 1:size(temperatureProfile, 1);
        x = (x - 1) * Algorithms.TM.samplingInterval;

        for i = 1:cores
          color = colors{mod(i - 1, length(colors)) + 1};
          line(x, temperatureProfile(:, i), 'Color', color);
        end
      end
    end

    function frequency = calculateFrequency(ssdtc, Vdd, Vbs)
      K1 = 0.063;
      K2 = 0.153;
      K6 = 5.26e-12;
      Ld = 10;
      Vth1 = 0.244;
      alpha = 1;

      frequency = ((1 + K1) * Vdd + K2 * Vbs - Vth1)^alpha / (K6 * Ld);
    end

    function power = calculateLeakagePower(ssdtc, Vdd, Vbs)
      K3 = 5.38e-7;
      K4 = 1.83;
      K5 = 4.19;
      Ij = 4.80e-10;

      power = Vdd * K3 * exp(K4 * Vdd) * exp(K5 * Vbs) - abs(Vbs) * Ij;
    end
  end
end
