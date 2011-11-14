function [ maxtab, mintab ] = peakdet(v, delta)
  maxtab = [];
  mintab = [];

  mn = v(1);
  mx = v(1);
  mnpos = 1;
  mxpos = 1;

  lookformax = false;

  for i = 2:length(v)
    this = v(i);

    if this > mx, mx = this; mxpos = i; end
    if this < mn, mn = this; mnpos = i; end

    if lookformax
      if this < (mx - delta)
        maxtab(end + 1, :) = [ mxpos mx ];
        mn = this;
        mnpos = i;
        lookformax = false;
      end
    else
      if this > (mn + delta)
        mintab(end + 1, :) = [ mnpos mn ];
        mx = this;
        mxpos = i;
        lookformax = true;
      end
    end
  end

  if ~lookformax
    if mintab(1, 2) > mn, mintab(1, 2) = mn; end
    return;
  end

  firstpos = maxtab(1, 1);
  found = false;

  for i = 1:(firstpos - 1)
    this = v(i);

    if this > mx, mx = this; mxpos = i; end

    if this >= (mx - delta), continue; end

    if mxpos < i
      maxtab = [ mxpos mx; maxtab ];
    else
      maxtab(end + 1, :) = [ mxpos mx ];
    end

    found = true;
    break;
  end

  if found, return; end

  if mintab(1, 2) < mintab(end, 2)
    mintab = mintab(1:(end - 1), :);
  else
    mintab = mintab(2:end, :);
  end
end
