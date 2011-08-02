function drawLines(labelTitle, labelX, labelY, x, y, index, varargin)
  if ~isempty(labelTitle), title(labelTitle); end
  if ~isempty(labelX), xlabel(labelX); end
  if ~isempty(labelY), ylabel(labelY); end
  if nargin < 6, index = []; end

  colors = Constants.roundRobinColors;

  for i = 1:size(y, 2)
    color = colors{mod(i - 1, length(colors)) + 1};
    if isempty(index)
      line(x, y(:, i), 'Color', color, varargin{:});
    else
      I = find(index(:, i));
      I = index(I, i);
      line(x(I), y(I, i), 'Color', color, varargin{:});
    end
  end
end