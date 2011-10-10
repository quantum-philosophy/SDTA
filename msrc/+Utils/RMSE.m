function rmse = RMSE(one, another)
  if nargin < 2, another = zeros(size(one)); end

  one = one(:);
  another = another(:);

  count = length(one);

  rmse = sqrt(sum((one - another) .^ 2) / count);
end
