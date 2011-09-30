factor = -0.1;

population_size = 300;
expectations = zeros(1, population_size);

sum = 0;

for i = 1:population_size
  expectations(i) = i ^ factor;
  sum = sum + expectations(i);
end

total = 0;

for i = 1:population_size
  expectations(i) = population_size * expectations(i) / sum;
  total = total + expectations(i);
end

plot(1:population_size, expectations);

fprintf('%5s%15s%15s\n', 'Rank', 'Probability', 'Cumulative');

F = 0;

for i = 1:population_size
  p = expectations(i) / total;
  F = F + p;
  fprintf('%5d%15.4f%15.4f\n', i, p, F);
end
