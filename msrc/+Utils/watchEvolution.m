function watchEvolution(file, multi)
  if nargin < 2, multi = false; end

  figure;

  title('Evolution');

  if multi
    xlabel('Lifetime');
    ylabel('Energy');
  else
    xlabel('Generation');
    ylabel('Lifetime');
  end

  t = timer('TimerFcn', @(a,b) disp('.'), 'StartDelay', 1);

  while 1
    lines = findobj(gca, 'Type', 'line');
    for i = 1:length(lines), delete(lines(i)); end

    Utils.drawEvolution(file, multi);

    start(t);
    wait(t);
  end
end
