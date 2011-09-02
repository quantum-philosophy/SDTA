function drawEvolution(file)
  evolution = dlmread(file);

  [ generations, population ] = size(evolution);

  figure;

  title('Evolution');

  for i = 1:generations
    line(ones(1, population) * i, evolution(i, :), ...
      'Line', 'None', 'Marker', 'x', 'Color', 'k');
  end

  line(1:generations, max(evolution, [], 2), 'Color', 'b');
end
