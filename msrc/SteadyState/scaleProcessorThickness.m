setup;

sweep = Sweep.Thickness('001_030', 0.10e-3:0.02e-3:0.50e-3);
sweep.perform();
sweep.draw();
