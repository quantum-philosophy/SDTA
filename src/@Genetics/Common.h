#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <list>

class Task;
class Graph;

class Processor;
class Architecture;

class ListScheduler;
class GeneticListScheduler;

typedef int tid_t;
typedef std::vector<tid_t> schedule_t;
typedef std::vector<Task *> task_vector_t;
typedef std::vector<double> priority_t;

typedef int pid_t;
typedef std::vector<pid_t> mapping_t;
typedef std::vector<Processor *> processor_vector_t;

#endif
