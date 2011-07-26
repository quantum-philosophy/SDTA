classdef TGFF < handle
  properties
    graphLabels
    tableLabels
    graphs
    tables
  end

  methods
    function tgff = TGFF(varargin)
      if nargin > 0, tgff.process(varargin{:}); end
    end

    function process(tgff, file, graphLabels, tableLabels)
      tgff.graphLabels = graphLabels;
      tgff.tableLabels = tableLabels;
      tgff.graphs = {};
      tgff.tables = {};

      fid = fopen(file);

      line = fgetl(fid);
      while ischar(line)
        attrs = regexp(line, '^@(\w+) (\d+) {$', 'tokens');
        if ~isempty(attrs)
          name = attrs{1}{1};
          id = str2num(attrs{1}{2});
          if Utils.include(tgff.graphLabels, name)
            graph = Graph(name, id);
            tgff.parseGraph(graph, fid);
            tgff.graphs{end + 1} = graph;
          elseif Utils.include(tgff.tableLabels, name)
            table = Table(name, id);
            tgff.parseTable(table, fid);
            tgff.tables{end + 1} = table;
          end
        end

        line = fgetl(fid);
      end

      fclose(fid);
    end
  end

  methods (Access = 'private')
    function parseGraph(tgff, graph, fid)
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
            graph.addTask(attrs{1}{:});

          case 'ARC'
            attrs = regexp(attrs, ...
              '(\w+)\s+FROM\s+(\w+)\s+TO\s+(\w+)\s+TYPE\s+(\d+)', 'tokens');
            graph.addLink(attrs{1}{:});

          case 'SOFT_DEADLINE'
            attrs = regexp(attrs, ...
              '(\w+)\s+ON\s+(\w+)\s+AT\s+(\d+\.?\d*)', 'tokens');
            graph.addDeadline(attrs{1}{:});

          case 'HARD_DEADLINE'
            attrs = regexp(attrs, ...
              '(\w+)\s+ON\s+(\w+)\s+AT\s+(\d+\.?\d*)', 'tokens');
            graph.addDeadline(attrs{1}{:});
          end
        end

        line = fgetl(fid);
      end
    end

    function parseTable(tgff, table, fid)
      state = State.SearchHeader;

      line = fgetl(fid);
      while ischar(line) && isempty(regexp(line, '^}$'))
        switch state
        case State.SearchHeader
          header = tgff.parseHeader(line);
          if ~isempty(header)
            if ~strcmp(header{1}, 'type')
              state = State.SearchTableAttributes;
            else
              state = State.SearchTypeHeader;
            end
          end

        case State.SearchTableAttributes
          attrs = sscanf(line, '%f');
          if length(attrs) == length(header)
            table.setAttributes(header, attrs);
            state = State.SearchTypeHeader;
          end

        case State.SearchTypeHeader
          header = tgff.parseHeader(line);
          if ~isempty(header) && strcmp(header{1}, 'type')
            table.setHeader(header(2:end));
            state = State.SearchTypeAttributes;
          end

        case State.SearchTypeAttributes
          attrs = sscanf(line, '%f');
          if length(attrs) == length(header)
            table.setRow(attrs(1) + 1, attrs(2:end));
          end
        end

        line = fgetl(fid);
      end
    end

    function header = parseHeader(tgff, line)
      header = {};
      chunks = regexp(line, '\s+', 'split');
      if ~isempty(chunks) && strcmp(chunks{1}, '#')
        header = chunks(2:end);
      end
    end
  end
end
