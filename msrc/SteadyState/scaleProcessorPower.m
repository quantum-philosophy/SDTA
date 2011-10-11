setup;

sweep = Sweep.Power('001_030', 1:5:100, 81e-6);
sweep.perform();
sweep.draw();
