function [ dlifetime, denergy ] = extractDominant(lifetime, energy)
  dlifetime = zeros(0);
  denergy = zeros(0);

  for i = 1:length(lifetime)
    dominant = true;

    for j = 1:length(lifetime)
      if i == j, continue; end

      if lifetime(i) < lifetime(j) && energy(i) > energy(j)
        dominant = false;
        break;
      end
    end

    if dominant
      dlifetime(end + 1) = lifetime(i);
      denergy(end + 1) = energy(i);
    end
  end
end
