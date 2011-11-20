setup;

sweep = Sweep.Steps('004', [ 50 100:100:1000 ] , { 'ce', 'fft', 'vta', 'vhs' });
sweep.perform();
sweep.draw();
