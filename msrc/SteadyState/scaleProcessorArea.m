setup;

dieArea = (1:25) * 1e-6;
convectionResistance = [ 0.1, 0.1, 0.1, 0.1, 0.1 ] * 5;
totalTime = [ 0.01, 0.02, 0.1, 0.5, 10 ];

areaCount = length(dieArea);
variantCount = length(convectionResistance);

figure;

Error = zeros(areaCount, variantCount);
legend = {};

for i = 1:variantCount
  sweep = Sweep.Area('001', dieArea, ...
    convectionResistance(i), totalTime(i));

  value = totalTime(i);

  sweep.perform();
  Error(1:end, i) = sweep.error;

  legend{end + 1} = [ 'Period = ', num2str(value * 1e3), ' ms' ];
end

options = struct( ...
  'xlabel', 'Processor Area, mm^2', ...
  'ylabel', 'Normalized RMSE, %', ...
  'marker', true ...
);

options.legend = legend;

Utils.draw(dieArea * 1e6, Error, options);

set(gca, 'YLim', [ 0, 1.1 * max(max(Error)) ]);
