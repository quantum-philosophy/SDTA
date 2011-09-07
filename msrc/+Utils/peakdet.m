function [ maxtab, mintab ] = peakdet(v, delta)
  maxtab = [];
  mintab = [];

  mn = v(1);
  mx = v(1);
  mnpos = 1;
  mxpos = 1;

  lookformax = true;

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

  % Go around through the first extremum to the second one
  if lookformax || isempty(maxtab), return; end
  nexti = maxtab(1, 1);

  for i = 1:(nexti - 1)
    this = v(i);

    if this < mn, mn = this; mnpos = i; end

    if this > (mn + delta)
      if mnpos > i
        mintab(end + 1, :) = [ mnpos mn ];
      else
        mintab = [ mnpos mn; mintab ];
      end
      break;
    end
  end
end
