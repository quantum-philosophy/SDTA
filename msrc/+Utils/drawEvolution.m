function drawEvolution(file, multi, point, full)
  if nargin < 4, full = false; end
  if nargin < 3, point = []; end
  if nargin < 2, multi = false; end

  evolution = dlmread(file);

  labels = {};

  if multi
    xlabel('Lifetime, time units');
    ylabel('Energy, J');

    [ generations, population ] = size(evolution);
    population = population / 2;

    I = (1:population) * 2 - 1;

    lifetime = evolution(:, I);
    energy = evolution(:, I + 1);

    if full
      line(lifetime(:), energy(:), ...
        'Line', 'None', 'Marker', 'x', 'Color', 'k');
      labels{end + 1} = 'History';

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
    labels{end + 1} = [ 'Max lifetime (', ...
      num2str(Utils.round2(maxLifetime, 0.01)), ')' ];

    line([ 0, maxLifetime ], [ minEnergy, minEnergy ], 'Line', '--', 'Color', 'k');
    labels{end + 1} = [ 'Min energy (', ...
      num2str(Utils.round2(minEnergy, 0.01)), ')' ];

    if length(point) == 2
      percentLifetime = Utils.round2((maxLifetime/point(1) - 1) * 100, 0.01);
      percentEnergy = Utils.round2((withEnergy/point(2) - 1) * 100, 0.01);
      title([ 'GA (LT ', num2str(percentLifetime), '%, E ', ...
        num2str(percentEnergy), '%)' ]);
    elseif length(point) == 1
      percentLifetime = Utils.round2((maxLifetime/point(1) - 1) * 100, 0.01);
      title([ 'GA (LT ', num2str(persentLifetime), '%)' ]);
    else
      title([ 'GA (LT ', num2str(Utils.round2(maxLifetime, 0.01)), ', E ', ...
        num2str(Utils.round2(withEnergy, 0.01)), ')' ]);
    end
  else
    xlabel('Generation');
    ylabel('Lifetime, time units');

    [ generations, population ] = size(evolution);

    if full
      steps = ones(size(evolution));

      for i = 1:generations
        steps(i, :) = i * steps(i, :);
      end

      line(steps(:), evolution(:), ...
        'Line', 'None', 'Marker', 'x', 'Color', 'k');
      labels{end + 1} = 'History';
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
    labels{end + 1} = [ 'Max lifetime (', num2str(maxLifetime), ')' ];

    line([ generations, generations ], [ 0, maxLifetime ], ...
      'Line', '-.', 'Color', 'k');
    labels{end + 1} = [ 'Last generation (', num2str(generations), ')' ];

    if isempty(point)
      title([ 'GA (LT ', num2str(maxLifetime), ')' ]);
    else
      percentLifetime = Utils.round2((maxLifetime/point - 1) * 100, 0.01);
      title([ 'GA (+', num2str(percentLifetime), '%)' ]);
    end
  end

  legend(labels{:});
end
