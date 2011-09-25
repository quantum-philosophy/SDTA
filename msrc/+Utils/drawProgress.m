function drawProgress(name, y)
  chunks = regexp(name, '^(.*), (.*)$', 'tokens');

  if ~isempty(chunks)
    title(chunks{1}{1});
  else
    title(name);
  end

  xlabel('Run');
  ylabel(name);

  mx = Utils.round2(max(y), 0.01);
  mn = Utils.round2(min(y), 0.01);
  av = Utils.round2(mean(y), 0.01);

  count = length(y);

  line(1:count, y, 'Color', 'k');

  line([1, count], [ mx, mx ], 'Line', '--', 'Color', 'r');
  line([1, count], [ av, av ], 'Line', '--', 'Color', 'g');
  line([1, count], [ mn, mn ], 'Line', '--', 'Color', 'b');

  set(gca, 'YLim', [ 0, 1.1 * mx ]);
  set(gca, 'XLim', [ 1, count ]);

  legend('Progress', ...
    [ 'Minimal (', num2str(mn), ')' ], ...
    [ 'Average (', num2str(av), ')' ], ...
    [ 'Maximal (', num2str(mx), ')' ], ...
    'Location', 'SouthEast');
end
