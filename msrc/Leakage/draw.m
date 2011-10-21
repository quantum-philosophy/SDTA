setup;

Ngate = 2e5;
Vdd = 1;

T = (20:100) + Constants.degreeKelvin;
P = Power.calculateStatic(Ngate, T, Vdd);

polynomial = polyfit(T, P, 1);

P2 = polyval(polynomial, T);

for i = 1:(length(polynomial) - 1)
  if i > 1, fprintf(' + '); end
  fprintf('%f * T^%d', polynomial(i), i);
end

fprintf(' + %f\n', polynomial(end));

plot(T, P, T, P2);
