setup;

sweep = Sweep.Steps('001_030', ...
  [ 0.05, 0.1, 0.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ] * 1000);
sweep.perform();
sweep.draw();
