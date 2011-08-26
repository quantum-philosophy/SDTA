#ifndef __TASK_GRAPH_H__
#define __TASK_GRAPH_H__

#include <vector>

class Task;
class Processor;
class Graph;

typedef std::vector<unsigned int> schedule_t;
typedef std::vector<Task *> task_vector_t;

typedef std::vector<unsigned int> mapping_t;
typedef std::vector<Processor *> processor_vector_t;

#include "Task.h"
#include "Processor.h"
#include "Graph.h"

#endif
