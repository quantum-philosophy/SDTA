function [ vdd, ngate ] = collectLeakageParams(graph)
  vdd = [];
  ngate = [];

  pes = graph.pes;

  for i = 1:length(pes)
    vdd(end + 1) = pes{i}.voltage;
    ngate(end + 1) = pes{i}.ngate;
  end
end
