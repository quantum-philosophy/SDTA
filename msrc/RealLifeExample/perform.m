setup;

config = Optima('#../../test/mpeg2/mpeg2', 3);

graph = config.taskGraph('criticality');
graph.inspect();
graph.draw();
