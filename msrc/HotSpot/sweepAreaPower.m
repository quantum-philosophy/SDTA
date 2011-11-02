setup;

chunks = 1:10:201;
totalTime = 1;
spreaderSide = 20e-3;
sinkSide = 30e-3;
sinkThickness = 10e-3;

config = Optima('001_030');

dieArea = [ 1, 4, 9, 16, 25 ] * 1e-6;
variants = [ dieArea', 2 * ones(5, 1) ];

chunkCount = length(chunks);
variantCount = size(variants, 1);

Error = zeros(chunkCount, variantCount);
legend = {};

fprintf('%20s%20s%20s%20s%20s%20s%20s%20s\n', ...
  'Area, mm^2', 'Die, mm', 'Spreader, mm', 'Sink, mm', ...
  'Pdyn, W', 'Ptot, W', 'Tmin, C', 'Tmax, C');

power = Optima.get_power(config.system, config.floorplan, ...
  config.hotspot, config.params, 'deadline_ratio 1');

[ stepCount, processorCount ] = size(power);
timeScale = totalTime / (stepCount * config.samplingInterval);

for k = 1:variantCount
  processorArea = variants(k, 1);
  maxPower = variants(k, 2);

  config.changeArea(processorArea);
  [ sinkSide, spreaderSide, dieSide ] = ...
    config.changePackage(spreaderSide, sinkSide, sinkThickness);

  powerScale = maxPower / max(max(power));

  param_line = @(solution, max_iterations) ...
    Utils.configStream(...
      'deadline_ratio', 1, ...
      'power_scale', powerScale, ...
      'time_scale', timeScale, ...
      'solution', solution, ...
      'max_iterations', max_iterations, ...
      'verbose', 0, ...
      'leakage', '');

  [ chunkTce, totalPower ] = Optima.solve(config.system, config.floorplan, ...
      config.hotspot, config.params, param_line('condensed_equation', 0));

  chunkTce = chunkTce - Constants.degreeKelvin;

  for i = 1:chunkCount
    chunkThs = Optima.solve(config.system, config.floorplan, config.hotspot, ...
      config.params, param_line('transient_analytical', i)) - Constants.degreeKelvin;

    Error(i, k) = Utils.NRMSE(chunkTce, chunkThs, 1) * 100;
  end

  side = sqrt(processorArea * 1e6);
  legend{end + 1} = sprintf('%.0f x %.0f mm^2', side, side);

  fprintf('%20.2f%20.2f%20.2f%20.2f%20.f%20.2f%20.2f%20.2f\n', ...
    processorArea * 1e6, dieSide * 1e3, spreaderSide * 1e3, sinkSide * 1e3, ...
    maxPower, max(max(totalPower)), ...
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
