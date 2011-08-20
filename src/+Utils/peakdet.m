function [ maxtab, mintab ] = peakdet(v, delta)
  maxtab = [];
  mintab = [];

  mn = Inf;
  mx = -Inf;
  mnpos = NaN;
  mxpos = NaN;

  lookformax = true;

  for i = 1:length(v)
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
  if maxtab(1, 1) < mintab(1, 1)
    lookforanothermax = true;
    if size(mintab, 1) < 2, return; end
    nexti = mintab(2, 1);
  else
    lookforanothermax = false;
    if size(maxtab, 1) < 2, return; end
    nexti = maxtab(2, 1);
  end

  for i = 1:(nexti - 1)
    this = v(i);

    if this > mx, mx = this; mxpos = i; end
    if this < mn, mn = this; mnpos = i; end

    if lookformax
      if this < (mx - delta)
        if lookforanothermax
          % Remove the first one, append to the end
          maxtab = [ maxtab(2:end, :); mxpos mx ];
        else
          % Append to the end
          maxtab(end + 1, :) = [ mxpos mx ];
        end
        break;
      end
    else
      if this > (mn + delta)
        if ~lookforanothermax
          % Remove the first one, append to the end
          mintab = [ mintab(2:end, :); mnpos mn ];
        else
          % Append to the end
          mintab(end + 1, :) = [ mnpos mn ];
        end
        break;
      end
    end
  end
end
