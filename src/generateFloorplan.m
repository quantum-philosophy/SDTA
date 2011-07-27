function names = generateFloorplan(file, cores, dimension)
  if nargin < 3, dimension = 0.005; end
  if nargin < 2, cores = 4; end

  row = floor(sqrt(cores));
  names = {};

  f = fopen(file, 'w');

  for i = 0:(cores - 1)
    x = mod(i, row) * dimension;
    y = floor(i / row) * dimension;
    names{i + 1} = sprintf('core%d', i);
    fprintf(f, '%s\t%f\t%f\t%f\t%f\n',...
      names{i + 1}, dimension, dimension, x, y);
  end

  fclose(f);
end
