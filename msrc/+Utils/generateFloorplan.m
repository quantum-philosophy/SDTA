function names = generateFloorplan(file, cores, dieSize)
  % Intel i7 620M processing die size 81mm^2
  if nargin < 3, dieSize = 81e-6; end

  inline = floor(sqrt(cores));
  dimension = sqrt(dieSize) / inline;
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
