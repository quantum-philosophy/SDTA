#include <stdexcept>
#include <iostream>
#include <string.h>

#include "Common.h"

std::ostream &operator<< (std::ostream &o, const std::vector<int> &vector)
{
	size_t size = vector.size();

	o << "[ ";
	for (size_t i = 0; i < size; i++) {
		o << vector[i];
		if (i < size - 1) o << ", ";
	}
	o << " ]" << std::endl;

	return o;
}
