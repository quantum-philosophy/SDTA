setup;

config = Optima('001_030');
hotspot = Hotspot(config.floorplan, config.hotspot, '');

P = zeros(10, 3);

A = hotspot.constructFull(P, 1e-3);

f = figure;

spy(A);
xlabel('');
ylabel('');
title('Sparseness of the System', 'FontSize', 16);

line = findobj(f, 'type', 'Line');
set(line, 'Color', Constants.roundRobinColors{1});
