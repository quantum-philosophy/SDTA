setup;

sweep = Sweep.Cores('001_030', [ 1, 4, 9, 16, 32, 64 ]);
sweep.perform();
sweep.draw();
