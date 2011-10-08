function drawLines(Title, XLabel, YLabel, x, Legend1, y1, Legend2, y2)
  title(Title, 'FontSize', 16);
  xlabel(XLabel, 'FontSize', 14);
  ylabel(YLabel, 'FontSize', 14);

  lineCount = size(y1, 2);
  colors = Constants.roundRobinColors;

  if lineCount == 1
    line(x, y2, 'Color', colors{2}, 'Line', '--');
    line(x, y1, 'Color', colors{1});
  else
    colorCount = length(colors);

    for i = 1:lineCount
      colorId = mod(i - 1, colorCount) + 1;

      line(x, y2(:, i), 'Color', colors{colorId}, 'Line', '--');
      line(x, y1(:, i), 'Color', colors{colorId});
    end
  end

  set(gca, 'XLim', [ x(1), x(end) ]);

  legend(Legend2, Legend1);
end
