function equalScales(rows, cols, col)
  XMin = Inf;
  XMax = -Inf;

  YMin = Inf;
  YMax = -Inf;

  for i = 1:rows
    subplot(rows, cols, (i - 1) * cols + col);

    XLim = get(gca, 'XLim');
    XMin = min(XMin, XLim(1));
    XMax = max(XMax, XLim(2));

    YLim = get(gca, 'YLim');
    YMin = min(YMin, YLim(1));
    YMax = max(YMax, YLim(2));
  end

  for i = 1:rows
    subplot(rows, cols, (i - 1) * cols + col);
    set(gca, 'XLim', [ XMin, XMax ]);
    set(gca, 'YLim', [ YMin, YMax ]);
  end
end
