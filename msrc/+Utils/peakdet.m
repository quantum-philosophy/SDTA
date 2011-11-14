function [ maxtab, mintab ] = peakdet(v, delta)
  maxtab = zeros(0, 2);
  mintab = zeros(0, 2);

  mn = Inf;
  mx = -Inf;
  mnpos = 0;
  mxpos = 0;

  UNDEFINED = 0;
  MIN = 1;
  MAX = 2;

  lookfor = UNDEFINED;
  firstis = UNDEFINED;

  for i = 1:length(v)
    this = v(i);

    if this >= mx, mx = this; mxpos = i; end
    if this <= mn, mn = this; mnpos = i; end

    if lookfor == MAX
      if this < (mx - delta)
        maxtab(end + 1, :) = [ mxpos mx ];
        mn = this;
        mnpos = i;
        lookfor = MIN;
      end
    elseif lookfor == MIN
      if this > (mn + delta)
        mintab(end + 1, :) = [ mnpos mn ];
        mx = this;
        mxpos = i;
        lookfor = MAX;
      end
    else
      if this < (mx - delta)
        maxtab(end + 1, :) = [ mxpos mx ];
        mn = this;
        mnpos = i;
        lookfor = MIN;
        firstis = MAX;
      elseif this > (mn + delta)
        mintab(end + 1, :) = [ mnpos mn ];
        mx = this;
        mxpos = i;
        lookfor = MAX;
        firstis = MIN;
      end
    end
  end

  if lookfor == MAX
    maxtab(end + 1, :) = [ mxpos mx ];

    if firstis == MIN && mintab(1, 1) > 1
      maxtab = [ maxtab; 1 mx ];
    end
  else
    mintab(end + 1, :) = [ mnpos mn ];

    if firstis == MAX && maxtab(1, 1) > 1
      mintab = [ mintab; 1 mn ];
    end
  end
end
