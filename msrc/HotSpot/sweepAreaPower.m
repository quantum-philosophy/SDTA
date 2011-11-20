setup;

chunks = [ 1 (5:5:50) ];

totalTime = 0.5;

config = Optima('001');

dieArea = [ 1, 4, 9, 16, 25 ] * 1e-6;
maxPower = [ 10, 20, 40, 60, 80 ];

chunkCount = length(chunks);
variantCount = length(dieArea);

Error = zeros(chunkCount, variantCount);
legend = {};

fprintf('%20s%20s%20s%20s%20s%20s%20s%20s%20s\n', ...
  'Area, mm^2', 'Die, mm', 'Spreader, mm', 'Sink, mm', 'Thickness, mm', ...
  'Pdyn, W', 'Ptot, W', 'Tmin, C', 'Tmax, C');

param_line = Utils.configStream('deadline_ratio', 1, 'verbose', 0);

power = Optima.get_power(config.system, config.floorplan, ...
  config.hotspot, config.params, param_line);

[ stepCount, processorCount ] = size(power);

timeScale = totalTime / (stepCount * config.samplingInterval);

for k = 1:variantCount
  processorArea = dieArea(k);

  config.changeArea(processorArea);
  [ sinkSide, spreaderSide, dieSide, sinkThickness ] = config.standardPackage();

  powerScale = maxPower(k) / max(max(power));

  param_line = @(solution, max_iterations) ...
    Utils.configStream(...
      'deadline_ratio', 1, ...
      'max_iterations', max_iterations, ...
      'tolerance', 0, ...
      'power_scale', powerScale, ...
      'time_scale', timeScale, ...
      'solution', solution, ...
      'verbose', 0, ...
      'leakage', '');

  [ chunkTce, totalPower ] = Optima.solve(config.system, config.floorplan, ...
      config.hotspot, config.params, param_line('condensed_equation', 0));

  chunkTce = chunkTce - Constants.degreeKelvin;

  for i = 1:chunkCount
    chunkThs = Optima.solve(config.system, config.floorplan, config.hotspot, ...
      config.params, param_line('transient_analytical', chunks(i))) - Constants.degreeKelvin;

    Error(i, k) = Utils.NRMSE(chunkTce, chunkThs) * 100;
  end

  legend{end + 1} = sprintf('Area = %.0f mm^2', processorArea * 1e6);

  fprintf('%20.2f%20.2f%20.2f%20.2f%20.2f%20.f%20.2f%20.2f%20.2f\n', ...
    processorArea * 1e6, dieSide * 1e3, spreaderSide * 1e3, sinkSide * 1e3, ...
    sinkThickness * 1e3, maxPower(k), max(max(totalPower)), ...
    min(min(chunkTce)), max(max(chunkTce)) ...
  );
end

figure;

options = struct( ...
  'xlabel', 'Iterations', ...
  'ylabel', 'Normalized RMSE, %', ...
  'marker', true);
options.legend = legend;

Utils.draw(chunks, Error, options);

set(gca, 'YLim', [ 0, 1.1 * max(max(Error)) ]);
