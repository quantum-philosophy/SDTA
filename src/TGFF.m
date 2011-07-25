classdef TGFF < handle
  properties
    graphLabels = { 'TASK_GRAPH' };
    tableLabels = { 'PE', 'COMMUN' };
    graphs = {};
    tables = {};
  end

  methods
    function tgff = TGFF(file)
      if nargin > 0
        tgff.parseFile(file)
      end
    end

    function parseFile(tgff, file)
      fid = fopen(file);

      line = fgetl(fid);
      while ischar(line)
        attrs = regexp(line, '^@(\w+) (\d+) {$', 'tokens');
        if ~isempty(attrs)
          name = attrs{1}{1};
          id = str2num(attrs{1}{2});
          if tgff.isGraph(name)
            graph = Graph(name, id);
            tgff.parseGraph(graph, fid);
            tgff.graphs{length(tgff.graphs) + 1} = graph;
          elseif tgff.isTable(name)
            table = Table(name, id);
            tgff.parseTable(table, fid);
            tgff.tables{length(tgff.tables) + 1} = table;
          end
        end

        line = fgetl(fid);
      end

      fclose(fid);
    end

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
      state = 0;

      line = fgetl(fid);
      while ischar(line) && ~tgff.isEnd(line)
        switch state
        case 0 % Looking for a header
          header = tgff.parseHeader(line);
          if ~isempty(header)
            if ~strcmp(header{1}, 'type')
              state = 1;
            else
              state = 3;
            end
          end

        case 1 % Looking for the table attributes
          attrs = sscanf(line, '%f');
          if length(attrs) == length(header)
            table.setAttributes(header, attrs);
            state = 2;
          end

        case 2 % Looking for the type header
          header = tgff.parseHeader(line);
          if ~isempty(header) && strcmp(header{1}, 'type')
            table.setHeader(header(2:end));
            state = 3;
          end

        case 3 % Reading the type attributes
          attrs = sscanf(line, '%f');
          if length(attrs) == length(header)
            table.setRow(attrs(1) + 1, attrs(2:end));
          end
        end

        line = fgetl(fid);
      end
    end

    function result = isIn(tgff, cells, name)
      result = 0;
      for i = 1:length(cells)
        if strcmp(cells{i}, name)
          result = 1;
          return;
        end
      end
    end

    function result = isGraph(tgff, name);
      result = tgff.isIn(tgff.graphLabels, name);
    end

    function result = isTable(tgff, name);
      result = tgff.isIn(tgff.tableLabels, name);
    end

    function result = isEnd(tgff, line)
      result = ~isempty(regexp(line, '^}$'));
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
