/*
  <qapPackUnpack.h>
  Copyright (C) DOLPHIN Project-Team, INRIA Lille Nord Europe, 2006-2009
  (C) OPAC Team, LIFL, 2002-2009

  The Van LUONG,  (The-Van.Luong@inria.fr)
  Mahmoud FATENE, (mahmoud.fatene@inria.fr)

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
/** 
 * Since you define your own class, you need to tell ParadisEO how to pack
 * and unpack your own data.
 * This example show you how we pack and unpack a QAP class. In our case, it is
 * just packing and unpacking an array of integers. In your case, it will depend
 * on the representation of your class.
 */
void pack( const Problem& _problem )
{

  if ( _problem.invalid() ) 
    pack( (unsigned int)0 );
  else
    {
      pack( (unsigned int)1 );
      pack(_problem.fitness());
    }

  for (int i = 0; i < n; i++ )
    {
      pack( _problem.solution[i] );
    }
}

void unpack( Problem& _problem )
{
  
  unsigned int validFitness;
  unpack( validFitness );
  if ( validFitness )
    {
      double fitnessValue;
      unpack( fitnessValue );
      _problem.fitness( fitnessValue );    
    }
  else
    {
      _problem.invalidate();
    }

  for (int i = 0; i < n; i++ )
    {
      unpack(_problem[i]);
    }
}
/*
void unpack( eoMinimizingFitness& __fitness )
{
   
  cout << " unpacking .... " << endl;
  
}
*/
