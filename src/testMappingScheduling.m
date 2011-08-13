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

mapping = Utils.generateEvenMapping(length(pes), graph.taskCount);
graph.assignMapping(pes, mapping);

schedule = [ 34, 37, 16, 42, 47, 8, 11, 44, 30, 3, 48, 50, 19, 25, 13, 4, 26, 20, 40, 45, 12, 49, 35, 36, 21, 28, 1, 32, 14, 6, 43, 31, 10, 22, 23, 24, 41, 51, 15, 17, 9, 2, 18, 46, 29, 5, 39, 38, 27, 33, 52, 7 ];

Utils.inspectVector('Schedule', schedule);

tic
graph.assignSchedule(schedule);
toc

graph.inspect();

Utils.drawMappingScheduling(graph);
