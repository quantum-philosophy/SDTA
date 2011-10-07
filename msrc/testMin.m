clc;

tasks = 3;
wnc = [ 2.85e6, 1e6, 4.3e6 ];
ceff = [ 1e-9, 0.9e-10, 1.5e-8 ];
deadline = 0.0128;
v = 1:0.1:1.8;
area = 7 * 7;
f = 638e6;
t = wnc ./ f;

fprintf('%10s', 'V');

for i = 1:tasks
  fprintf('%10s', [ 'P', num2str(i) ]);
end

for i = 1:tasks
  fprintf('%10s', [ 'E', num2str(i) ]);
end

fprintf('\n');

for i = 1:length(v)
  power = ceff .* f .* v(i)^2;
  energy = power .* t;
  fprintf('%10.3f', v(i));
  fprintf('%10.3f', power / area);
  fprintf('%10.3f', energy);
  fprintf('\n');
end
