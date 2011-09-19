% Test: Ensure that scheduling in time works fine

clear all;
clc;
rng(0);

name = '004_060';
floorplan = Utils.path([ name, '.flp' ]);
testCase = Utils.path([ name, '.tgff' ]);
config = Utils.path('hotspot.config');

tgff = TestCase.TGFF(testCase);
graph = tgff.graphs{1};
pes = tgff.pes;

hotspot = HotSpot(floorplan, config);

% Mapping
mapping = Utils.generateEvenMapping(length(pes), length(graph.tasks));
graph.assignMapping(pes, mapping);

% Scheduling
schedule = LS.process(pes, graph, mapping);
graph.assignDistributedSchedule(schedule);

graph.assignDeadline(Constants.deadlineFactor * graph.duration);

graph.inspect();
graph.draw();
