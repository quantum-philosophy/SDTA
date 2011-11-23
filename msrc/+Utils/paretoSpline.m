function s = paretoSpline(lifetime, energy, N)
  mn = min(lifetime);
  mx = max(lifetime);
  d = (mx - mn) / (N - 1);
  x = mn:d:mx;

  [ lifetime, I ] = sort(lifetime);
  energy = energy(I);

  s = zeros(1, N);

  for i = 1:N
    for j = 1:length(lifetime)
      if x(i) == lifetime(j)
        s(i) = energy(j);
        break;
      elseif x(i) < lifetime(j)
        k = (energy(j) - energy(j - 1)) / (lifetime(j) - lifetime(j - 1));
        b = energy(j) - k * lifetime(j);
        s(i) = k * x(i) + b;
        break;
      end
    end
  end

  if length(s) ~= N
    error('Invalid');
  end
end
