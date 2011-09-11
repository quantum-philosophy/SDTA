function drawEvolution(file, multi)
  evolution = dlmread(file);

  if multi
    [ generations, population ] = size(evolution);
    population = population / 2;

    I = (1:population) * 2 - 1;

    lifetime = evolution(:, I);
    energy = evolution(:, I + 1);

    line(lifetime(:), energy(:), ...
      'Line', 'None', 'Marker', 'x', 'Color', 'k');

    line(lifetime(end, :), energy(end, :), ...
      'Line', 'None', 'Marker', 'o', 'Color', 'r');

    [ lifetime, I ] = sort(lifetime(end, :));
    energy = energy(end, I);

    [ lifetime, energy ] = Utils.extractDominant(lifetime, energy);

    line(lifetime, energy, 'Color', 'b');

    maxLifetime = max(lifetime);
    minEnergy = min(energy);

    xlim = get(gca, 'XLim');
    ylim = get(gca, 'YLim');

    line([ maxLifetime, maxLifetime ], ylim, 'Line', '-.', 'Color', 'k');
    line(xlim, [ minEnergy, minEnergy ], 'Line', '--', 'Color', 'k');

    legend([ 'History (', num2str(generations), ' generations)' ], ...
      'Last population', 'Pareto front', ...
      [ 'Max lifetime (', num2str(maxLifetime), ' time units)' ], ...
      [ 'Min energy (', num2str(minEnergy), ' J)' ]);
  else
    [ generations, population ] = size(evolution);

    steps = ones(size(evolution));

    for i = 1:generations
      steps(i, :) = i * steps(i, :);
    end

    line(steps(:), evolution(:), ...
      'Line', 'None', 'Marker', 'x', 'Color', 'k');

    line(1:generations, max(evolution, [], 2), 'Color', 'b');

    maxLifetime = max(max(evolution));

    xlim = get(gca, 'XLim');

    line(xlim, [ maxLifetime, maxLifetime ], 'Line', '--', 'Color', 'k');

    legend([ 'History (', num2str(generations), ' generations)' ], ...
      'Evolution', ...
      [ 'Max lifetime (', num2str(maxLifetime), ' time units)' ]);
  end
end
