setup;

sweep = Sweep.Cores('001_030', ...
  [ 1, 4, 9, 16, 32, 64, 128, 256, 512 ], { 'ce', 'ta', 'hs' });
sweep.perform();
sweep.draw();
