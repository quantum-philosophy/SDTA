clear all;
clc;

tgff = TGFF('simple.tgff');
ls = LS();

for i = 1:length(tgff.graphs)
  graph = tgff.graphs{i};
  graph.inspect();
  ls.process(graph);
  ls.inspect();
end
