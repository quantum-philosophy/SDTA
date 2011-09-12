function watchEvolution(file, multi, point, full)
  if nargin < 2, multi = false; end
  if nargin < 3, point = []; end
  if nargin < 4, full = true; end

  figure;

  t = timer('TimerFcn', @(a,b)1, 'StartDelay', 1);

  while 1
    lines = findobj(gca, 'Type', 'line');
    for i = 1:length(lines), delete(lines(i)); end

    try
      Utils.drawEvolution(file, multi, point, full);
    catch
    end

    start(t);
    wait(t);
  end
end
