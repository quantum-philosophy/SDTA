setup;

processorArea = 4e-6;
nominalStepCount = 1000;
powerPerCore = 12;

fprintf('%15s%15s%15s%15s%15s%15s\n', 'Cores', 'Steps', 'Equations', 'NNZ', 'Sparseness', 'Time, s');

config = Optima('001_030');

for processorCount = [ 1, 4, 9, 16, 32, 64 ]
  ratio = (4 * 1 + 12) / (4 * processorCount + 12);

  stepCount = round(ratio * nominalStepCount);

  config.changeProcessorCountAndArea(processorCount, processorArea);
  config.scalePackage();

  hotspot = Hotspot(config.floorplan, config.hotspot, '');
  power = Power.generateRandomProfile( ...
    processorCount, stepCount, processorCount * powerPerCore);

  [ A, B ] = hotspot.constructBand(power, hotspot.samplingInterval);
  nm = size(A, 1);
  nz = nnz(A);

  fprintf('%15d%15d%15d%15d%15.4f', processorCount, stepCount, nm, nz, 1 - nz / (nm * nm));

  tic;
  hotspot.solve(power, 'band');
  t = toc;

  fprintf('%15.2f\n', t);
end
