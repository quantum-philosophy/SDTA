function showEvolutionStatsSet(prefix)
  files = dir([ prefix, '/', '*.log' ]);

  realizationCount = length(files);
  realizationCount = 9;

  figure;

  rows = floor(sqrt(realizationCount));
  cols = ceil(realizationCount / rows);

  ylims = [];

  for i = 1:realizationCount
    subplot(rows, cols, i);
    file = files(i);
    file = sprintf('%s/%s', prefix, file.name);
    disp(file);
    Utils.showEvolutionStats(file, true);
    ylims = [ ylims; get(gca, 'YLim') ];
  end

  ymin = min(ylims(:, 1));
  ymax = max(ylims(:, 2));

  for i = 1:realizationCount
    subplot(rows, cols, i);
    set(gca, 'YLim', [ ymin, ymax ]);
  end
end
