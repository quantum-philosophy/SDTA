function drawEvolutionSet(prefix, multi, point, full)
  if nargin < 2, multi = false; end
  if nargin < 3, point = []; end
  if nargin < 4, full = false; end

  files = dir([ prefix, '/', 'evolution.txt_*' ]);

  try
    Utils.showEvolutionStats([ prefix, '/', 'evolution.log' ]);
  catch
  end

  evolutionCount = length(files);

  figure;

  rows = floor(sqrt(evolutionCount));
  cols = ceil(evolutionCount / rows);

  xlims = [];
  ylims = [];

  for i = 1:evolutionCount
    subplot(rows, cols, i);
    file = [ prefix, '/', 'evolution.txt_', num2str(i - 1) ];
    Utils.drawEvolution(file, multi, point, full);

    legend 'off';

    xlims = [ xlims; get(gca, 'XLim') ];
    ylims = [ ylims; get(gca, 'YLim') ];
  end

  xmin = min(xlims(:, 1));
  xmax = max(xlims(:, 2));
  ymin = min(ylims(:, 1));
  ymax = max(ylims(:, 2));

  for i = 1:evolutionCount
    subplot(rows, cols, i);
    set(gca, 'XLim', [ xmin, xmax ], 'YLim', [ ymin, ymax ]);
  end
end
