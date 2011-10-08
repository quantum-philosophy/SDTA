function stream = configStream(varargin)
  stream = '';

  for i = 1:nargin
    term = varargin{i};

    if isnumeric(term)
      stream = [ stream, num2str(term) ];
    else
      stream = [ stream, term ];
    end

    if mod(i - 1, 2) == 0
      stream = [ stream, ' ' ];
    elseif i < nargin
      stream = [ stream, sprintf('\n') ];
    end
  end
end
