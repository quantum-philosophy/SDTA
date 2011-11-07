setup;

sweep = Sweep.Cores('001_030', ...
  [ 1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 ], { 'ce', 'hs' });
sweep.perform();
sweep.draw();

legend('Condensed Equation Method', 'One Transient Temperature Simulation');
