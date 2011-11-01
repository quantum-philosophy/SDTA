setup;

dieArea = (1:3:81) * 1e-6;

spreaderNominalRatio = 21 / 3.99;

convectionResistance = [ 0.1, 0.2, 0.3, 0.4, 0.5 ];
powerDensity = [ 40, 40, 40, 40, 40 ] * 1e4;
spreaderRatio = ones(1, 5) * spreaderNominalRatio;

areaCount = length(dieArea);
variantCount = length(convectionResistance);

figure;

Error = zeros(areaCount, variantCount);
legend = {};

for i = 1:variantCount
  sweep = Sweep.Area('001_030', dieArea, ...
    convectionResistance(i), powerDensity(i), spreaderRatio(i));

  value = convectionResistance(i);

  sweep.perform();
  Error(1:end, i) = sweep.error;
  legend{end + 1} = [ 'R_{conv} = ', num2str(value), ' K/W' ];
end

options = struct( ...
  'title', 'Steady-State Error', ...
  'xlabel', 'Die area, mm^2', ...
  'ylabel', 'Normalized RMSE, %', ...
  'marker', true ...
);

options.legend = legend;

Utils.draw(dieArea * 1e6, Error, options);
