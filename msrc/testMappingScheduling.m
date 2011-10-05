% Test: Ensure that scheduling in time works fine

clear all;
clc;
rng(0);

name = '002_030';
floorplan = Utils.path([ name, '.flp' ]);
testCase = Utils.path([ name, '.tgff' ]);
config = Utils.path('hotspot.config');

tgff = TestCase.TGFF(testCase);
graph = tgff.graphs{1};
pes = tgff.pes;

hotspot = HotSpot(floorplan, config);

LS.mapEarliestAndSchedule(graph, pes);

graph.assignDeadline(Constants.deadlineFactor * graph.duration);

graph.inspect();
graph.draw();
