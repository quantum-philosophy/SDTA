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

Ta = T(1);
Tb = T(end);
Tc = (Tb + Ta) / 2;

Pa = Power.calculateStatic(Ngate, Ta, Vdd);
Pb = Power.calculateStatic(Ngate, Tb, Vdd);
Pc = Power.calculateStatic(Ngate, Tc, Vdd);

k = (Pb - Pa) / (Tb - Ta);
b1 = Pa - k * Ta;
b2 = Pc - k * Tc;
b = (b1 + b2) / 2;

plot(T, P, T, P2, T, k * T + b);
