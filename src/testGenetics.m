clear all;
clc;
rng(0);

name = 'test_cases/test_case_4_60';

testCase = Utils.path([ name, '.tgff' ]);
floorplan = Utils.path([ name, '.flp' ]);
config = Utils.path('hotspot.config');

tgff = TestCase.TGFF(testCase);
graph = tgff.graphs{1};
processors = tgff.pes;

mapping = Utils.generateEvenMapping(length(processors), length(graph.tasks));
graph.assignMapping(processors, mapping);

graph.inspect();

pack = Utils.compactTaskGraph(graph, processors);

Genetics.optimizeAging(floorplan, config, pack.type - 1, pack.link, ...
  pack.frequency, pack.voltage, pack.ngate, pack.nc, pack.ceff);
