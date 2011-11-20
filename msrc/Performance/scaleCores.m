setup;

sweep = Sweep.Cores('001', [ 2, 4, 8, 16, 32 ], { 'ce', 'fft', 'vta', 'vhs' });
sweep.perform();
sweep.draw();
