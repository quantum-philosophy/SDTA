classdef TGFF < handle
  properties (Constant, Access = private)
    SearchHeader = 0;
    SearchTableAttributes = 1;
    SearchTypeHeader = 2;
    SearchTypeAttributes = 3;
  end

  properties (SetAccess = private)
    graphs
    pes
  end

  methods
    function tgff = TGFF(file, graphLabels, peLabels)
      if nargin < 2, graphLabels = Constants.graphLabels; end
      if nargin < 3, peLabels = Constants.peLabels; end

      graphs = {};
      pes = {};

      fid = fopen(file);

      line = fgetl(fid);
      while ischar(line)
        attrs = regexp(line, '^@(\w+) (\d+) {$', 'tokens');
        if ~isempty(attrs)
          name = attrs{1}{1};
          type = str2num(attrs{1}{2});
          if Utils.include(graphLabels, name)
            id = length(graphs) + 1;
            graph = TestCase.Graph(id, name, type);
            TestCase.TGFF.parseGraph(graph, fid);
            graphs{end + 1} = graph;
          elseif Utils.include(peLabels, name)
            id = length(pes) + 1;
            pe = TestCase.Processor(id, name, type);
            TestCase.TGFF.parseProcessor(pe, fid);
            pes{end + 1} = pe;
          end
        end

        line = fgetl(fid);
      end

      fclose(fid);

      tgff.graphs = graphs;
      tgff.pes = pes;
    end
  end

  methods (Access = private, Static)
    function parseGraph(graph, fid)
      line = fgetl(fid);
      while ischar(line) && isempty(regexp(line, '^}$'))
        attrs = regexp(line, '^\s*(\w+)\s+(.*)$', 'tokens');

        if ~isempty(attrs)
          command = attrs{1}{1};
          attrs = attrs{1}{2};

          switch command
          case 'PERIOD'
            graph.assignPeriod(str2num(attrs));

          case 'TASK'
            attrs = regexp(attrs, '(\w+)\s+TYPE\s+(\d+)', 'tokens');
            if ~isempty(attrs)
              attrs = attrs{1};
              % Counting from 1 instead of 0
              attrs{2} = str2num(attrs{2}) + 1;
              graph.addTask(attrs{:});
            end

          case 'ARC'
            attrs = regexp(attrs, ...
              '(\w+)\s+FROM\s+(\w+)\s+TO\s+(\w+)\s+TYPE\s+(\d+)', 'tokens');
            if ~isempty(attrs)
              attrs = attrs{1};
              % Counting from 1 instead of 0
              attrs{4} = str2num(attrs{4}) + 1;
              graph.addLink(attrs{:});
            end

          case 'HARD_DEADLINE'
            attrs = regexp(attrs, ...
              '(\w+)\s+ON\s+(\w+)\s+AT\s+(\d+\.?\d*)', 'tokens');
            attrs = attrs{1};
            attrs{3} = str2num(attrs{3});
            if ~isempty(attrs), graph.assignDeadline(attrs{:}); end
          end
        end

        line = fgetl(fid);
      end
    end

    function parseProcessor(pe, fid)
      state = TestCase.TGFF.SearchHeader;

      ceffI = 0;
      ncI = 0;

      line = fgetl(fid);
      while ischar(line) && isempty(regexp(line, '^}$'))
        switch state
        case TestCase.TGFF.SearchHeader
          header = TestCase.TGFF.parseHeader(line);
          if ~isempty(header)
            if ~strcmp(header{1}, 'type')
              state = TestCase.TGFF.SearchTableAttributes;
            else
              state = TestCase.TGFF.SearchTypeHeader;
            end
          end

        case TestCase.TGFF.SearchTableAttributes
          attrs = sscanf(line, '%f');
          if length(attrs) == length(header)
            for i = 1:length(header)
              name = header{i};
              if strcmp(name, 'frequency')
                pe.frequency = attrs(i);
              elseif strcmp(name, 'voltage')
                pe.voltage = attrs(i);
              elseif strcmp(name, 'ngate')
                pe.ngate = attrs(i);
              else
                error('Found an unknown attribute for a processing element');
              end
            end
            state = TestCase.TGFF.SearchTypeHeader;
          end

        case TestCase.TGFF.SearchTypeHeader
          header = TestCase.TGFF.parseHeader(line);
          if ~isempty(header) && strcmp(header{1}, 'type')
            typeIndes = [];
            for i = 2:length(header)
              if strcmp(header{i}, 'effective_switched_capacitance')
                ceffI = i;
              elseif strcmp(header{i}, 'number_of_clock_cycles')
                ncI = i;
              end
            end
            if ~ceffI || ~ncI
              error('Cannot find necessary type attributes');
            end
            state = TestCase.TGFF.SearchTypeAttributes;
          end

        case TestCase.TGFF.SearchTypeAttributes
          attrs = sscanf(line, '%f');
          if length(attrs) == length(header)
            % Counting from 1 instead of 0
            pe.addType(attrs(1) + 1, attrs(ceffI), attrs(ncI));
          end
        end

        line = fgetl(fid);
      end
    end

    function header = parseHeader(line)
      header = {};
      chunks = regexp(line, '\s+', 'split');
      if ~isempty(chunks) && strcmp(chunks{1}, '#')
        header = chunks(2:end);
      end
    end
  end
end
