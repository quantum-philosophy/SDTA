function generateFloorplan(file, processorCount, processorArea)
  if nargin < 3, processorArea = 4e-6; end

  inline = floor(sqrt(processorCount));
  processorSide = sqrt(processorArea);

  fid = fopen(file, 'w');

  for i = 0:(processorCount - 1)
    x = mod(i, inline) * processorSide;
    y = floor(i / inline) * processorSide;

    fprintf(fid, '%s\t%f\t%f\t%f\t%f\n',...
      [ 'core', num2str(i + 1) ], processorSide, processorSide, x, y);
  end

  fclose(fid);
end
