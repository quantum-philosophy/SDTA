function [ dlifetime, denergy ] = extractDominant(lifetime, energy)
  ulifetime = zeros(0);
  uenergy = zeros(0);

  for i = 1:length(lifetime)
    unique = true;

    for j = (i + 1):length(lifetime)
      if lifetime(i) == lifetime(j) && energy(i) == energy(j)
        unique = false;
        break;
      end
    end

    if unique
      ulifetime(end + 1) = lifetime(i);
      uenergy(end + 1) = energy(i);
    end
  end

  dlifetime = zeros(0);
  denergy = zeros(0);

  for i = 1:length(ulifetime)
    dominant = true;

    for j = 1:length(ulifetime)
      if i == j, continue; end

      if ulifetime(i) < ulifetime(j) && uenergy(i) > uenergy(j)
        dominant = false;
        break;
      end
    end

    if dominant
      dlifetime(end + 1) = ulifetime(i);
      denergy(end + 1) = uenergy(i);
    end
  end
end
