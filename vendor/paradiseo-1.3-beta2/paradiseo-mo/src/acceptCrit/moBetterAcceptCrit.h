/*
<moBetterAcceptCrit.h>
Copyright (C) DOLPHIN Project-Team, INRIA Lille - Nord Europe, 2006-2010

Sébastien Verel, Arnaud Liefooghe, Jérémie Humeau

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  ue,
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.
The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.

ParadisEO WebSite : http://paradiseo.gforge.inria.fr
Contact: paradiseo-help@lists.gforge.inria.fr
*/

#ifndef _moBetterAcceptCrit_h
#define _moBetterAcceptCrit_h

#include <acceptCrit/moAcceptanceCriterion.h>
#include <memory/moDummyMemory.h>
#include <comparator/moSolComparator.h>

/**
 * Acceptance Criterion for extreme intensification : accept if the new solution is better than previous one
 */
template< class Neighbor >
class moBetterAcceptCrit : public moAcceptanceCriterion<Neighbor>, public moDummyMemory<Neighbor> {

public:
    typedef typename Neighbor::EOT EOT;

    moBetterAcceptCrit(moSolComparator<EOT>& _comparator):comparator(_comparator) {}

    /**
     * Accept if the new solution is better than previous one
     * @param _sol1 the previous solution
     * @param _sol2 the new solution after local search
     * @return true if the new solution is better than previous one
     */
    bool operator()(EOT& _sol1, EOT& _sol2) {
        return comparator(_sol1, _sol2);
    }

private:
    moSolComparator<EOT>& comparator;

};

#endif
