classdef TGFFParser < TestCase.Parser
  methods
    function parser = TGFFParser(varargin)
      parser@TestCase.Parser(varargin{:});
    end

    function process(parser, file, graphLabels, tableLabels)
      parser.graphs = {};
      parser.tables = {};

      fid = fopen(file);

      line = fgetl(fid);
      while ischar(line)
        attrs = regexp(line, '^@(\w+) (\d+) {$', 'tokens');
        if ~isempty(attrs)
          name = attrs{1}{1};
          id = str2num(attrs{1}{2});
          if Utils.include(graphLabels, name)
            graph = TestCase.Graph(name, id);
            parser.parseGraph(graph, fid);
            parser.graphs{end + 1} = graph;
          elseif Utils.include(tableLabels, name)
            table = TestCase.Table(name, id);
            parser.parseTable(table, fid);
            parser.tables{end + 1} = table;
          end
        end

        line = fgetl(fid);
      end

      fclose(fid);
    end
  end

  methods (Access = private)
    function parseGraph(parser, graph, fid)
      line = fgetl(fid);
      while ischar(line) && isempty(regexp(line, '^}$'))
        attrs = regexp(line, '^\s*(\w+)\s+(.*)$', 'tokens');

        if ~isempty(attrs)
          command = attrs{1}{1};
          attrs = attrs{1}{2};

          switch command
          case 'PERIOD'
            graph.setAttribute('period', str2num(attrs));

          case 'TASK'
            attrs = regexp(attrs, ...
              '(\w+)\s+TYPE\s+(\d+)', 'tokens');
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

          case 'SOFT_DEADLINE'
            attrs = regexp(attrs, ...
              '(\w+)\s+ON\s+(\w+)\s+AT\s+(\d+\.?\d*)', 'tokens');
            if ~isempty(attrs), graph.addDeadline(attrs{1}{:}); end

          case 'HARD_DEADLINE'
            attrs = regexp(attrs, ...
              '(\w+)\s+ON\s+(\w+)\s+AT\s+(\d+\.?\d*)', 'tokens');
            if ~isempty(attrs), graph.addDeadline(attrs{1}{:}); end
          end
        end

        line = fgetl(fid);
      end
    end

    function parseTable(parser, table, fid)
      state = TestCase.TGFFState.SearchHeader;

      line = fgetl(fid);
      while ischar(line) && isempty(regexp(line, '^}$'))
        switch state
        case TestCase.TGFFState.SearchHeader
          header = parser.parseHeader(line);
          if ~isempty(header)
            if ~strcmp(header{1}, 'type')
              state = TestCase.TGFFState.SearchTableAttributes;
            else
              state = TestCase.TGFFState.SearchTypeHeader;
            end
          end

        case TestCase.TGFFState.SearchTableAttributes
          attrs = sscanf(line, '%f');
          if length(attrs) == length(header)
            table.setAttributes(header, attrs);
            state = TestCase.TGFFState.SearchTypeHeader;
          end

        case TestCase.TGFFState.SearchTypeHeader
          header = parser.parseHeader(line);
          if ~isempty(header) && strcmp(header{1}, 'type')
            table.setHeader(header(2:end));
            state = TestCase.TGFFState.SearchTypeAttributes;
          end

        case TestCase.TGFFState.SearchTypeAttributes
          attrs = sscanf(line, '%f');
          if length(attrs) == length(header)
            % Counting from 1 instead of 0
            table.setRow(attrs(1) + 1, attrs(2:end));
          end
        end

        line = fgetl(fid);
      end
    end

    function header = parseHeader(parser, line)
      header = {};
      chunks = regexp(line, '\s+', 'split');
      if ~isempty(chunks) && strcmp(chunks{1}, '#')
        header = chunks(2:end);
      end
    end
  end
end
