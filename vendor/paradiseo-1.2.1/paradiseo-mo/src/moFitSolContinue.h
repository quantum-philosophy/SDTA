/*
  <moFitSolContinue.h>
  Copyright (C) DOLPHIN Project-Team, INRIA Futurs, 2006-2008
  (C) OPAC Team, LIFL, 2002-2008
 
  Sébastien Cahon, Jean-Charles Boisson (Jean-Charles.Boisson@lifl.fr)
 
  This software is governed by the CeCILL license under French law and
  abiding by the rules of distribution of free software.  You can  use,
  modify and/ or redistribute the software under the terms of the CeCILL
  license as circulated by CEA, CNRS and INRIA at the following URL
  "http://www.cecill.info".
 
  As a counterpart to the access to the source code and  rights to copy,
  modify and redistribute granted by the license, users are provided only
  with a limited warranty  and the software's author,  the holder of the
  economic rights,  and the successive licensors  have only  limited liability.
 
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

#ifndef _moFitSolContinue_h
#define _moFitSolContinue_h

#include <moSolContinue.h>

//! One possible stop criterion for a solution-based heuristic.
/*!
  The stop criterion corresponds to a fitness threshold gained.
*/
template < class EOT >
class moFitSolContinue:public moSolContinue < EOT >
{
 public:

  //! Alias for the fitness.
  typedef typename EOT::Fitness Fitness;

  //! Basic constructor.
  /*!
    \param _fitness The fitness to reach.
  */
  moFitSolContinue (Fitness _fitness): fitness(_fitness)
  {}

  //! Function that activates the stopping criterion.
  /*!
    Indicates if the fitness threshold has not yet been reached.

    \param _solution the current solution.
    \return true or false according to the value of the fitness.
  */
  bool operator () (const EOT & _solution)
  {
    if ( _solution.invalid() )
      {
       	throw std::runtime_error("[moFitSolContinue.h]: The current solution has not been evaluated.");
      }

    return fitness > _solution.fitness();
  }

  //! Procedure which allows to initialise all the stuff needed.
  /*!
    It can be also used to reinitialize all the needed things.
  */
  void init ()
  {}

 private:

  //! Fitness target.
  Fitness fitness;
};

#endif
