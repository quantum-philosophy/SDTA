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

    minLifetime = min(min(lifetime));
    maxLifetime = max(max(lifetime));
    minEnergy = min(min(energy));
    maxEnergy = max(max(energy));

    [ lifetime, I ] = sort(lifetime(end, :));
    energy = energy(end, I);

    [ lifetime, energy ] = Utils.extractDominant(lifetime, energy);

    line(lifetime, energy, 'Color', 'b');
    labels{end + 1} = 'Pareto front';

    if ~full
      minLifetime = lifetime(1);
      maxLifetime = lifetime(end);
      minEnergy = min(energy);
      maxEnergy = max(energy);
    end

    withEnergy = energy(end);

    set(gca, 'XLim', [ minLifetime, maxLifetime ], ...
      'YLim', [ minEnergy, maxEnergy ]);

    line([ maxLifetime, maxLifetime ], [ 0, maxEnergy ], 'Line', '-.', 'Color', 'k');
    labels{end + 1} = [ 'Max lifetime (', num2str(maxLifetime), ' time units)' ];

    line([ 0, maxLifetime ], [ minEnergy, minEnergy ], 'Line', '--', 'Color', 'k');
    labels{end + 1} = [ 'Min energy (', num2str(minEnergy), ' J)' ];

    if length(point) == 2
      percentLifetime = Utils.round2((maxLifetime/point(1) - 1) * 100, 0.01);
      percentEnergy = Utils.round2((withEnergy/point(2) - 1) * 100, 0.01);
      title([ 'GA (LT ', num2str(percentLifetime), '%, E ', ...
        num2str(percentEnergy), '%)' ]);
    elseif length(point) == 1
      percentLifetime = Utils.round2((maxLifetime/point(1) - 1) * 100, 0.01);
      title([ 'GA (LT ', num2str(persentLifetime), '%)' ]);
    else
      title([ 'GA (max ', num2str(maxLifetime), ' TU with ', ...
        num2str(withEnergy), ' J)' ]);
    end
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

    if isempty(point)
      title([ 'GA (max ', num2str(maxLifetime), ')' ]);
    else
      percentLifetime = Utils.round2((maxLifetime/point - 1) * 100, 0.01);
      title([ 'GA (+', num2str(percentLifetime), '%)' ]);
    end
  end

  legend(labels{:});
end
