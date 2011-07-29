function inspectVector(name, vector)
  fprintf('%s: [ ', name);
  for i = 1:length(vector)
    if i ~= 1, fprintf(', %d', vector(i));
    else fprintf('%d', vector(i));
    end
  end
  fprintf(' ]\n');
end
