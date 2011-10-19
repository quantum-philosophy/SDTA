setup;

sweep = Sweep.Cores('001_030', (1:16).^2);
sweep.perform();
sweep.draw();
