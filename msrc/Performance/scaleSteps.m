setup;

sweep = Sweep.Steps('004_060', ...
  [ 0.05, 0.1, 0.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ] * 1000, ...
  { 'ce', 'hs' });
sweep.perform();
sweep.draw();

legend('Condensed Equation Method', 'One Transient Temperature Simulation');
