function names = generateFloorplan(file, cores, dimension)
  % 1cm x 1cm the whole chip
  if nargin < 3, dimension = 0.01; end
  if nargin < 2, cores = 4; end

  inline = floor(sqrt(cores));
  dimension = dimension / inline;
  names = {};

  f = fopen(file, 'w');

  for i = 0:(cores - 1)
    x = mod(i, inline) * dimension;
    y = floor(i / inline) * dimension;
    names{i + 1} = sprintf('core%d', i + 1);
    fprintf(f, '%s\t%f\t%f\t%f\t%f\n',...
      names{i + 1}, dimension, dimension, x, y);
  end

  fclose(f);
end
