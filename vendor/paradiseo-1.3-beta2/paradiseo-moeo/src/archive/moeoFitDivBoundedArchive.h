/*
* <moeoFitDivBoundedArchive.h>
* Copyright (C) DOLPHIN Project-Team, INRIA Lille-Nord Europe, 2006-2008
* (C) OPAC Team, LIFL, 2002-2008
*
* Arnaud Liefooghe
* Jeremie Humeau
*
* This software is governed by the CeCILL license under French law and
* abiding by the rules of distribution of free software.  You can  use,
* modify and/ or redistribute the software under the terms of the CeCILL
* license as circulated by CEA, CNRS and INRIA at the following URL
* "http://www.cecill.info".
*
* As a counterpart to the access to the source code and  rights to copy,
* modify and redistribute granted by the license, users are provided only
* with a limited warranty  and the software's author,  the holder of the
* economic rights,  and the successive licensors  have only  limited liability.
*
* In this respect, the user's attention is drawn to the risks associated
* with loading,  using,  modifying and/or developing or reproducing the
* software by the user in light of its specific status of free software,
* that may mean  that it is complicated to manipulate,  and  that  also
* therefore means  that it is reserved for developers  and  experienced
* professionals having in-depth computer knowledge. Users are therefore
* encouraged to load and test the software's suitability as regards their
* requirements in conditions enabling the security of their systems and/or
* data to be ensured and,  more generally, to use and operate it in the
* same conditions as regards security.
* The fact that you are presently reading this means that you have had
* knowledge of the CeCILL license and that you accept its terms.
*
* ParadisEO WebSite : http://paradiseo.gforge.inria.fr
* Contact: paradiseo-help@lists.gforge.inria.fr
*
*/
//-----------------------------------------------------------------------------
// moeoFitDivBoundedArchive.h
//-----------------------------------------------------------------------------

#ifndef MOEOFITDIVBOUNDEDARCHIVE_H_
#define MOEOFITDIVBOUNDEDARCHIVE_H_

#include <list>
#include <eoPop.h>
#include <comparator/moeoComparator.h>
#include <comparator/moeoObjectiveVectorComparator.h>
#include <diversity/moeoDiversityAssignment.h>
#include <fitness/moeoFitnessAssignment.h>

/**
 * This class represents a bounded archive which different parameters to specify.
 */
template < class MOEOT >
class moeoFitDivBoundedArchive : public moeoBoundedArchive < MOEOT >
{
public:

    using moeoArchive < MOEOT > :: size;
    using moeoArchive < MOEOT > :: resize;
    using moeoArchive < MOEOT > :: operator[];
    using moeoArchive < MOEOT > :: back;
    using moeoArchive < MOEOT > :: pop_back;
    using moeoArchive < MOEOT > :: push_back;
    using moeoArchive < MOEOT > :: begin;
    using moeoArchive < MOEOT > :: end;
    using moeoArchive < MOEOT > :: replace;
    using moeoBoundedArchive < MOEOT > :: maxSize;


    /**
     * The type of an objective vector for a solution
     */
    typedef typename MOEOT::ObjectiveVector ObjectiveVector;


    /**
     * Ctor where you can choose your own moeoComparator, moeoObjectiveVectorComparator, moeoFitnessAssignment, moeoDiversityAssignment and archive size.
     * @param _indiComparator the functor used to compare MOEOT
     * @param _comparator the functor used to compare objective vectors
     * @param _fitness the assignment fitness method
     * @param _diversity the diversity assignment method
     * @param _maxSize the size of archive (must be smaller or egal to the population size)
     * @param _replace boolean which determine if a solution with the same objectiveVector than another one, can replace it or not
     */
    moeoFitDivBoundedArchive(moeoComparator < MOEOT > & _indiComparator, moeoObjectiveVectorComparator < ObjectiveVector > & _comparator, moeoFitnessAssignment < MOEOT > & _fitness, moeoDiversityAssignment < MOEOT > & _diversity, unsigned int _maxSize=100, bool _replace=true) : moeoBoundedArchive < MOEOT >(_comparator, _maxSize, _replace), indiComparator(_indiComparator), fitness(_fitness), diversity(_diversity)
    {}

    /**
     * Ctor with moeoParetoObjectiveVectorComparator where you can choose your own moeoComparator, moeoFitnessAssignment, moeoDiversityAssignment and archive size.
     * @param _indiComparator the functor used to compare MOEOT
     * @param _fitness the assignment fitness method
     * @param _diversity the diversity assignment method
     * @param _maxSize the size of archive (must be smaller or egal to the population size)
     * @param _replace boolean which determine if a solution with the same objectiveVector than another one, can replace it or not
     */
    moeoFitDivBoundedArchive(moeoComparator < MOEOT > & _indiComparator, moeoFitnessAssignment < MOEOT > & _fitness, moeoDiversityAssignment < MOEOT > & _diversity, unsigned int _maxSize=100, bool _replace=true) : moeoBoundedArchive < MOEOT >(_maxSize, _replace), indiComparator(_indiComparator), fitness(_fitness), diversity(_diversity)
    {}

    /**
     * Updates the archive with a given individual _moeo
     * @param _moeo the given individual
     * @return true if _moeo is non-dominated (and not if it is added to the archive)
     */
    bool operator()(const MOEOT & _moeo)
    {
		bool res;
    	res = update(_moeo);

    	if(size() > maxSize){
    		fitness(*this);
    		diversity(*this);
    		std::sort(begin(), end(), indiComparator);
    		resize(maxSize);
    	}
    	return res;
    }


    /**
     * Updates the archive with a given population _pop
     * @param _pop the given population
     * @return true if a _pop[i] is non-dominated (and not if it is added to the archive)
     */
    bool operator()(const eoPop < MOEOT > & _pop)
    {
    	bool res;
    	res = update(_pop);

    	if(size() > maxSize){
    		fitness(*this);
    		diversity(*this);
    		std::sort(begin(), end(), indiComparator);
    		resize(maxSize);
    	}
    	return res;
    }

private:
    /**
     * Wrapper which allow to used an moeoComparator in std::sort
     * @param _comp the comparator to used
     */
    class Wrapper
    {
    public:
        /**
         * Ctor.
         * @param _comp the comparator
         */
        Wrapper(moeoComparator < MOEOT > & _comp) : comp(_comp) {}
        /**
         * Returns true if _moeo1 is greater than _moeo2 according to the comparator
         * _moeo1 the first individual
         * _moeo2 the first individual
         */
        bool operator()(const MOEOT & _moeo1, const MOEOT & _moeo2)
        {
            return comp(_moeo1,_moeo2);
        }
    private:
        /** the comparator */
        moeoComparator < MOEOT > & comp;
    }
    indiComparator;

    /** fitness assignment */
    moeoFitnessAssignment < MOEOT > & fitness;
    /** diversity assignment */
    moeoDiversityAssignment < MOEOT > & diversity;

};

#endif /*MOEOFITDIVBOUNDEDARCHIVE_H_*/
