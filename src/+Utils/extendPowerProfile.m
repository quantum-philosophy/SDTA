function extendPowerProfile(infile, outfile, repeat)
  % Read the power profile
  %
  in = fopen(infile, 'r');

  names = fgets(in);
  values = fread(in);

  fclose(in);

  % Write an extended version
  %
  out = fopen(outfile, 'w');

  fprintf(out, '%s', names);

  for i = 1:repeat, fwrite(out, values); end

  fclose(out);
end
