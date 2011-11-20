setup;

sweep = Sweep.Steps('001', [ 50 100:100:1000 ] , { 'ce', 'fft', 'vta', 'vhs' });
sweep.perform();
sweep.draw();
