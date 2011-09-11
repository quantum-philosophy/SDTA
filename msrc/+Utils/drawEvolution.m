function drawEvolution(file, multi)

  evolution = dlmread(file);

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

    [ lifetime, energy ] = Utils.extractDominant(lifetime, energy);

    line(lifetime, energy, ...
      'Marker', 'o', 'Color', 'b');
  else
    [ generations, population ] = size(evolution);

    for i = 1:generations
      line(ones(1, population) * i, evolution(i, :), ...
        'Line', 'None', 'Marker', 'x', 'Color', 'k');
    end

    line(1:generations, max(evolution, [], 2), 'Color', 'b');
  end
end
