/*
  <moItRandNextMove.h>
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

#ifndef _moItRandNextMove_h
#define _moItRandNextMove_h

#include <moNextMove.h>
#include <moRandMove.h>

//! One of the possible moNextMove.
/*!
  This class is a move (moMove) generator with a bound for the maximum number of iterations.
*/
template < class M > 
class moItRandNextMove:public moNextMove < M >
{
  //! Alias for the type.
  typedef typename M::EOType EOT;

 public:

  //! The constructor.
  /*!
    Parameters only for initialising the attributes.

    \param _random_move_generator The random move generator.
    \param _iteration_maximum_number The iteration maximum number.
  */
  moItRandNextMove (moRandMove < M > & _random_move_generator, unsigned int _iteration_maximum_number):
  random_move_generator(_random_move_generator), iteration_maximum_number(_iteration_maximum_number), iteration_number(0)
    {}

  //! Generation of a new move
  /*!
    If the maximum number is not already reached, the current move is forgotten and remplaced by another one.

    \param _move the current move.
    \param _solution the current solution.
    \return false if the maximum number of iteration is reached, else true.
  */
  bool operator () (M & _move, const EOT & _solution)
  {
    //code only used to avoid warning because _solution is not used in this function.
    const EOT solution(_solution);
    
    if (iteration_number > iteration_maximum_number)
      {
	iteration_number = 0;
	return false;
      }
    
    random_move_generator (_move);
    iteration_number++;
    
    return true;
  }

 private:

  //! A move generator (generally randomly).
  moRandMove < M > & random_move_generator;

  //! Iteration maximum number.
  unsigned int iteration_maximum_number;

  //! Iteration current number.
  unsigned int iteration_number;
};

#endif
