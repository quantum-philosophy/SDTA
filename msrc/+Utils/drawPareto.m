function drawPareto(lifetime, energy, extended)
  if nargin < 3, extended = true; end

  colors = Constants.roundRobinColors;

  mnL = lifetime(1);
  mxL = lifetime(end);
  mnE = energy(1);
  mxE = energy(end);

  figure;

  xlabel('MTTF, times', 'FontSize', 14);
  ylabel('Energy, times', 'FontSize', 14);
  addpath([docroot '/techdoc/creating_plots/examples']);

  line(lifetime, energy, 'Color', colors{1});

  if extended
    d = (mxL - mnL) / 100;
    x = mnL:d:mxL;
    p = polyfit(lifetime, energy, 3);
    line(x, polyval(p, x), 'Color', colors{2});
    line(lifetime, energy, 'Color', colors{3}, 'Line', 'none', 'Marker', 'o');

    legend('Pareto Front', 'Interpolated Pareto Front', 'Non-dominant Solutions');
  else
    legend('Pareto Front');
  end

  mnX = mnL - 0.1 * (mxL - mnL);
  mxX = 0.05 * (mxL - mnL) + mxL;

  mnY = mnE - 0.1 * (mxE - mnE);
  mxY = 0.05 * (mxE - mnE) + mxE;

  set(gca, 'XLim', [ mnX, mxX ]);
  set(gca, 'YLim', [ mnY, mxY ]);

  dL = (mxL - mnL) / mnL * 100;
  dE = (mxE - mnE) / mnE * 100;

  [ x, y ] = dsxy2figxy(gca, [ mnL, mnL ] - 0.05 * (mxL - mnL), [ mnE, mxE ]);
  annotation('doublearrow', x, y, ...
    'Head1Style', 'vback3', 'Head2Style', 'vback3', 'LineStyle', '--');
  text(mnL, (mxE + mnE) / 2, sprintf('%.2f%%', dE));

  [ x, y ] = dsxy2figxy(gca, [ mnL, mxL ], [ mnE, mnE ] - 0.05 * (mxE - mnE));
  annotation('doublearrow', x, y, ...
    'Head1Style', 'vback3', 'Head2Style', 'vback3', 'LineStyle', '--');
  text((mnL + mxL) / 2, mnE, sprintf('%.2f%%', dL));
end
