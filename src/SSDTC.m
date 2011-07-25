clear all;
clc;

tgff = TGFF('simple.tgff');

for i = 1:length(tgff.graphs)
  graph = tgff.graphs{i};
  fprintf('Task graph: %s %d\n', graph.name, graph.id);

  tasks = graph.getStartTasks();
end
