setup;

Ngate = 2e5;
Vdd = 0.9;

T1 = 30 + Constants.degreeKelvin;
T2 = 80 + Constants.degreeKelvin;
N = 20;

T = zeros(1, N);
for i = 1:N
  T(i) = T1 + (i - 1) * (T2 - T1) / N;
end

P = Power.calculateStatic(Ngate, T, Vdd);

polynomial = polyfit(T, P, 1);
P2 = polyval(polynomial, T);

n = length(T);

sxy = sum(T .* P);
sx = sum(T);
sy = sum(P);
ssx = sum(T .^ 2);

b1 = (n * sxy - sx * sy) / (n * ssx - sx^2);
b2 = (sy - b1 * sx) / n;

fprintf('P = %f * T + %f\n', b1, b2);

figure;
line(T, P, 'Color', 'k');
line(T, P2, 'Color', 'r', 'Marker', 'x');
line(T, b1 * T + b2, 'Color', 'g');
