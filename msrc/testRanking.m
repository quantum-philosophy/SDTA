factor = -0.25;

population_size = 300;
expectations = zeros(1, population_size);

s = 0;

for i = 1:population_size
  expectations(i) = i ^ factor;
  s = s + expectations(i);
end

total = 0;

for i = 1:population_size
  expectations(i) = population_size * expectations(i) / s;
  total = total + expectations(i);
end

plot(1:population_size, expectations);

fprintf('%5s%15s%15s%15s\n', 'Rank', 'Expectation', 'Probability', 'Cumulative');

F = 0;

for i = 1:population_size
  p = expectations(i) / total;
  F = F + p;
  fprintf('%5d%15.4f%15.4f%15.4f\n', i, expectations(i), p, F);
end
