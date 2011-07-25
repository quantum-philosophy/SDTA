clear all;
clc;

tic
tgff = TGFF('simple.tgff');
toc

for i = 1:length(tgff.graphs)
  graph = tgff.graphs{i};
  graph.inspect();

  tic
  ls = LS(graph);
  toc
  ls.inspect();
end
