function drawEvolution(file, multi, point, full)
  evolution = dlmread(file);

  labels = {};

  if multi
    xlabel('Lifetime');
    ylabel('Energy');

    [ generations, population ] = size(evolution);
    population = population / 2;

    I = (1:population) * 2 - 1;

    lifetime = evolution(:, I);
    energy = evolution(:, I + 1);

    if full
      line(lifetime(:), energy(:), ...
        'Line', 'None', 'Marker', 'x', 'Color', 'k');
      labels{end + 1} = [ 'History (', num2str(generations), ' generations)' ];

      line(lifetime(end, :), energy(end, :), ...
        'Line', 'None', 'Marker', 'o', 'Color', 'r');
      labels{end + 1} = 'Last population';
    end

    [ lifetime, I ] = sort(lifetime(end, :));
    energy = energy(end, I);

    minLifetime = min(min(lifetime));
    maxEnergy = max(max(energy));

    [ lifetime, energy ] = Utils.extractDominant(lifetime, energy);

    line(lifetime, energy, 'Color', 'b');
    labels{end + 1} = 'Pareto front';

    maxLifetime = max(lifetime);
    minEnergy = min(energy);

    if ~full
      minLifetime = min(lifetime);
      maxEnergy = max(energy);
    end

    set(gca, 'XLim', [ 0, maxLifetime ], 'YLim', [ 0, maxLifetime ]);

    line([ maxLifetime, maxLifetime ], [ 0, maxEnergy ], 'Line', '-.', 'Color', 'k');
    labels{end + 1} = [ 'Max lifetime (', num2str(maxLifetime), ' time units)' ];

    line([ 0, maxLifetime ], [ minEnergy, minEnergy ], 'Line', '--', 'Color', 'k');
    labels{end + 1} = [ 'Min energy (', num2str(minEnergy), ' J)' ];
  else
    xlabel('Generation');
    ylabel('Lifetime');

    [ generations, population ] = size(evolution);

    if full
      steps = ones(size(evolution));

      for i = 1:generations
        steps(i, :) = i * steps(i, :);
      end

      line(steps(:), evolution(:), ...
        'Line', 'None', 'Marker', 'x', 'Color', 'k');
      labels{end + 1} = [ 'History (', num2str(generations), ' generations)' ];
    end

    evolutionLine = max(evolution, [], 2);
    line(1:generations, evolutionLine, 'Color', 'b');
    labels{end + 1} = 'Evolution';

    maxLifetime = max(max(evolution));

    if full
      minLifetime = min(min(evolution));
    else
      minLifetime = min(evolutionLine);
    end

    set(gca, 'XLim', [ 1, generations ]);
    set(gca, 'YLim', [ minLifetime, maxLifetime ]);

    line([ 1, generations ], [ maxLifetime, maxLifetime ], ...
      'Line', '--', 'Color', 'k');
    labels{end + 1} = [ 'Max lifetime (', num2str(maxLifetime), ' time units)' ];

    line([ generations, generations ], [ 0, maxLifetime ], ...
      'Line', '-.', 'Color', 'k');
    labels{end + 1} = [ 'Max generations (', num2str(generations), ')' ];
  end

  if isempty(point)
    title([ 'GLS (max ', num2str(maxLifetime), ')' ]);
  else
    title([ 'GLS (+', num2str(round((maxLifetime/point - 1) * 100)), '%)' ]);
  end

  legend(labels{:});
end
