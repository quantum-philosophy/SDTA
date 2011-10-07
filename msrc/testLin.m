clc;

% ARM11

fprintf('ARM11\n');
fprintf('%-45s%20s%20s%20s%20s\n', 'Processor', 'Frequency, MHz', ...
  'Area, mm^2', 'Dynamic Power, W', 'Power, W/mm^2');

name = { '76JZ(F)-S TSMC65LP', '76JZ(F)-S TSMC 65GP',
  '76JZ(F)-S TSMC 40G', 'MPCore TSMC 65LP' };

frequency = [ 482, 772, 990, 427 ]; % MHz
area = [ 1.75, 1.94, 1.17, 3.26 ]; % mm^2

dynamic_power = [ 0.41, 0.208, 0.105, 0.318 ]; % mW/MHz
dynamic_power = dynamic_power .* frequency; % mW
dynamic_power = dynamic_power * 1e-3; % W

for i = 1:length(frequency)
  fprintf('%-45s%20d%20.3f%20.3f%20.3f\n', name{i}, frequency(i), ...
    area(i), dynamic_power(i), dynamic_power(i) / area(i));
end

% Cortex-A

fprintf('\n');
fprintf('Cortex-A\n');
fprintf('%-45s%20s%20s%20s%20s\n', 'Processor', 'Frequency, MHz', ...
  'Area, mm^2', 'Total Power, W', 'Power, W/mm^2');

name = { 'Single Core TSMC 65G', ...
  'Dual Core TSMC 40G (Performance Optimized)', ...
  'Dual Core TSMC 40G (Power Opttimized)' };

frequency = [ 830, 2000, 800 ]; % MHz
area = [ 1.5, 6.7, 4.6 ]; % mm^2
total_power = [ 0.4, 1.9, 0.5 ]; % W

for i = 1:length(frequency)
  fprintf('%-45s%20d%20.3f%20.3f%20.3f\n', name{i}, frequency(i), ...
    area(i), total_power(i), total_power(i) / area(i));
end

% Lin

fprintf('\n');
fprintf('Lin\n');

name = { 'Minimal', 'Maximal' };

power_density = [ 3.33, 12.5 ]; % W/cm^2
power_density = power_density * 1e-2; % W/mm^2

fprintf('%-45s%20s\n', '-', 'Power, W/mm^2');

for i = 1:length(power_density)
  fprintf('%-45s%20.3f\n', name{i}, power_density(i));
end
