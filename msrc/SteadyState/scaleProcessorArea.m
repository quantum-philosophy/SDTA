setup;

sweep = Sweep.Area('001_030', (2:10).^2 * 1e-6);
sweep.perform();
sweep.draw();
