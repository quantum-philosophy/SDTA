setup;

b = 2;
n = 1e4;
T = 1;

discrete = @(k) exp(-(k ./ n).^b);
continuous = @(t) exp(-((t ./ T) ./ n).^b);

mn = 0;
mx = 100;
cycles = mn:5:mx;
time = (mn:mx) * T;

figure;

line(cycles * T, discrete(cycles), 'Color', 'r');
line(time, continuous(time), 'Color', 'b');

mttf1 = n * gamma(1 + 1 / b);

mttf2 = 0;
k = 0;
while true
  value = discrete(k) * T;
  mttf2 = mttf2 + value;
  if value < eps, break; end
  k = k + 1;
end

fprintf('MTTF1: %.2f\n', mttf1);
fprintf('MTTF2: %.2f\n', mttf2);
fprintf('Relative error: %.2f%%\n', (mttf2 / mttf1 - 1) * 100);
