function drawEvolution(file, multi)
  if nargin < 2, multi = false; end

  evolution = dlmread(file);

  figure;

  title('Evolution');

  if multi
    [ generations, population ] = size(evolution);
    population = population / 2;

    I = (1:population) * 2 - 1;

    lifetime = evolution(:, I);
    energy = evolution(:, I + 1);

    for i = 1:generations
      line(lifetime(i, :), energy(i, :), ...
        'Line', 'None', 'Marker', 'x', 'Color', 'k');
    end

    [ lifetime, I ] = sort(lifetime(end, :));
    energy = energy(end, I);

    line(lifetime, energy, ...
      'Line', 'None', 'Marker', 'o', 'Color', 'b');
  else
    [ generations, population ] = size(evolution);

    for i = 1:generations
      line(ones(1, population) * i, evolution(i, :), ...
        'Line', 'None', 'Marker', 'x', 'Color', 'k');
    end

    line(1:generations, max(evolution, [], 2), 'Color', 'b');
  end
end
