setup;

dieArea = (1:25) * 1e-6;
convectionResistance = [ 0.1, 0.2, 0.3, 0.4, 0.5 ];

areaCount = length(dieArea);
variantCount = length(convectionResistance);

figure;

Error = zeros(areaCount, variantCount);
legend = {};

for i = 1:variantCount
  sweep = Sweep.Area('001_030', dieArea, convectionResistance(i));
  value = convectionResistance(i);

  sweep.perform();
  Error(1:end, i) = sweep.error;
  legend{end + 1} = [ 'R_{conv} = ', num2str(value), ' K/W' ];
end

options = struct( ...
  'xlabel', 'Processor Area, mm^2', ...
  'ylabel', 'Normalized RMSE, %', ...
  'marker', true ...
);

options.legend = legend;

Utils.draw(dieArea * 1e6, Error, options);
