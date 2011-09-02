function mapping = generateEvenMapping(cores, tasks)
  mapping = zeros(1, tasks);
  k = 1;
  for i = 1:tasks
    mapping(i) = k;
    k = k + 1;
    if k > cores, k = 1; end
  end
end
