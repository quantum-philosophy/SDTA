function drawSimulation(graph, powerProfile, T)
  if nargin < 3, plots = 2;
  else plots = 3;
  end

  steps = size(powerProfile, 1);

  figure;

  % Mapping and scheduling
  subplot(plots, 1, 1);
  graph.draw(false);

  x = ((1:steps) - 1) * Constants.samplingInterval;

  % Power profile
  subplot(plots, 1, 2);
  Utils.drawLines('Dynamic Power Profile', 'Time, s', 'Power, W', x, powerProfile);
  line(x, sum(powerProfile, 2), 'Color', 'k', 'Line', '--');
  set(gca, 'XLim', [ 0 graph.deadline ]);

  if nargin < 3, return; end

  % Temperature profile
  subplot(plots, 1, 3);
  T = T - Constants.degreeKelvin;
  Utils.drawLines('Temperature Profile', 'Time, s', 'Temperature, C', x, T);
  set(gca, 'XLim', [ 0 graph.deadline ]);
end
