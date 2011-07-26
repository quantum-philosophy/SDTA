classdef SSDTC < handle
  properties (SetAccess = private)
    % Heavy objects
    graphs
    pes
    comms

    % To speed up the access to the properties of the PEs
    voltage
    frequency
    ceff
    nc
  end

  properties (Access = private)
    scheduler
  end

  methods
    function ssdtc = SSDTC(varargin)
      ssdtc.scheduler = GLSA();

      if nargin > 0, ssdtc.process(varargin{:}); end
    end

    function process(ssdtc, file, graphLabel, peLabel, commLabel)
      if nargin < 3, graphLabel = 'TASK_GRAPH'; end
      if nargin < 4, peLabel = 'PE'; end
      if nargin < 5, commLabel = 'COMMUN'; end

      % Parse the TGFF configuration file
      tgff = TGFF(file, { graphLabel }, { peLabel commLabel });

      % Graphs
      ssdtc.graphs = tgff.graphs;

      % Tables
      ssdtc.pes = {};
      ssdtc.comms = {};
      for i = 1:length(tgff.tables)
        table = tgff.tables{i};
        if strcmp(table.name, peLabel), ssdtc.addPE(table);
        elseif strcmp(table.name, commLabel), ssdtc.addComm(table);
        end
      end

      for graph = ssdtc.graphs, ssdtc.calculate(graph{1}); end
    end

    function inspect(ssdtc)
      for graph = ssdtc.graphs, graph{1}.inspect(); end
      for pe = ssdtc.pes, pe{1}.inspect(); end
      for comm = ssdtc.comms, comm{1}.inspect(); end
    end
  end

  methods (Access = private)
    function calculate(ssdtc, graph)
      [ solution, fitness, flag ] = ssdtc.scheduler.process(graph, ssdtc.pes);
    end

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
        if strcmp(pe.header{i}, 'Ceff')
          ceff = pe.values(:, i);
        elseif strcmp(pe.header{i}, 'NC')
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
      ssdtc.pes{end + 1} = pe;
      ssdtc.voltage(end + 1) = pe.attributes('voltage');
      ssdtc.frequency(end + 1) = pe.attributes('frequency');
      ssdtc.ceff(end + 1, 1:length(ceff)) = ceff;
      ssdtc.nc(end + 1, 1:length(nc)) = nc;
    end

    function addComm(ssdtc, comm)
      ssdtc.comms{end + 1} = comm;
    end
  end
end
