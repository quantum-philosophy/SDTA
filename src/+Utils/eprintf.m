function eprintf(varargin)
  fprintf('ERROR: ');
  fprintf(varargin{:});
  fprintf('\n');
end
