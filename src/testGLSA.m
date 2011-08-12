% Test: Testdrive for the genetic algorithm for scheduling

clear all;
clc;

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

graph.inspect();

glsa = GLSA();

Utils.startTimer('Solve with GLSA');
priority = glsa.solve(graph, hotspot);
Utils.stopTimer();
