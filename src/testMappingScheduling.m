% Test: Ensure that scheduling in time works fine

clear all;
clc;
rng(0);

name = 'test_cases/test_case_4_60';

floorplan = Utils.path([ name, '.flp' ]);
testCase = Utils.path([ name, '.tgff' ]);
config = Utils.path('hotspot.config');

tgff = TestCase.TGFF(testCase);
graph = tgff.graphs{1};
pes = tgff.pes;

hotspot = HotSpot(floorplan, config);

mapping = Utils.generateEvenMapping(length(pes), length(graph.tasks));
graph.assignMapping(pes, mapping);

priority = randperm(length(graph.tasks));
schedule = LS.schedule(graph, priority);

Utils.inspectVector('Schedule', schedule);

tic
graph.assignSchedule(schedule);
toc

graph.inspect();

Utils.drawMappingScheduling(graph);
