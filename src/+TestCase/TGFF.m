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
          id = str2num(attrs{1}{2});
          if Utils.include(graphLabels, name)
            graph = TestCase.Graph(name, id);
            TestCase.TGFF.parseGraph(graph, fid);
            graphs{end + 1} = graph;
          elseif Utils.include(peLabels, name)
            pe = TestCase.Processor(name, id);
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
            graph.setPeriod(str2num(attrs));

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
            if ~isempty(attrs), graph.setDeadline(attrs{:}); end
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
            found = zeros(1, 3);
            for i = 1:length(header)
              name = header{i};
              if strcmp(name, 'frequency')
                found(1) = 1;
                pe.setFrequency(attrs(i));
              elseif strcmp(name, 'voltage')
                found(2) = 1;
                pe.setVoltage(attrs(i));
              elseif strcmp(name, 'ngate')
                found(3) = 1;
                pe.setNgate(attrs(i));
              else
                error('Found an unknown attribute for a processing element');
              end
            end
            if ~all(found)
              error('Cannot find all necessary attributes for a processing element');
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
