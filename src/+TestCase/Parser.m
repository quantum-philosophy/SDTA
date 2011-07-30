classdef Parser < handle
  properties (SetAccess = protected)
    graphs
    tables
  end

  methods (Abstract)
    process(parser, file)
  end

  methods
    function parser = Parser(varargin)
      if nargin > 0, parser.process(varargin{:}); end
    end
  end
end
