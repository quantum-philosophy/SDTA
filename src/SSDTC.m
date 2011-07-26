classdef SSDTC < handle
  properties
    graphs
    processors
    communications
  end

  methods
    function ssdtc = SSDTC(varargin)
      if nargin > 0, ssdtc.process(varargin{:}); end
    end

    function process(ssdtc, file, graphLabel, peLabel, comLabel)
      if nargin < 2, file = 'simple.tgff'; end
      if nargin < 3, graphLabel = 'TASK_GRAPH'; end
      if nargin < 4, peLabel = 'PE'; end
      if nargin < 5, comLabel = 'COMMUN'; end

      % Parse the TGFF configuration file
      tic
      tgff = TGFF(file, { graphLabel }, { peLabel comLabel });
      toc

      ssdtc.graphs = tgff.graphs;
      ssdtc.processors = {};
      ssdtc.communications = {};

      for i = 1:length(tgff.tables)
        table = tgff.tables{i};
        if strcmp(table.name, peLabel)
          ssdtc.processors{end + 1} = table;
        elseif strcmp(table.name, comLabel)
          ssdtc.communications{end + 1} = table;
        end
      end

      for i = 1:length(ssdtc.graphs)
        ssdtc.calculate(ssdtc.graphs{i});
      end
    end
  end

  methods (Access = 'private')
    function calculate(ssdtc, graph)
      graph.inspect();
      tic
      ls = LS(graph);
      toc
      ls.inspect();
    end
  end
end
