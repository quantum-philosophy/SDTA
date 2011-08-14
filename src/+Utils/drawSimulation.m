function drawSimulation(graph, powerProfile)
  steps = size(powerProfile, 1);

  figure;

  % Mapping and scheduling
  subplot(2, 1, 1);
  Utils.drawMappingScheduling(graph);

  % Power profile
  subplot(2, 1, 2);
  x = ((1:steps) - 1) * Constants.samplingInterval;
  Utils.drawLines('Dynamic Power Profile', 'Time, s', 'Power, W', x, powerProfile);
  line(x, sum(powerProfile, 2), 'Color', 'k', 'Line', '--');
end
