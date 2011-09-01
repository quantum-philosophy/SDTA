// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-
 
//-----------------------------------------------------------------------------
// eoESMute.h : ES mutation
// (c) Maarten Keijzer 2000 & GeNeura Team, 1998 for the EO part
//     Th. Baeck 1994 and EEAAX 1999 for the ES part
/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
 
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    Contact: todos@geneura.ugr.es, http://geneura.ugr.es
             marc.schoenauer@polytechnique.fr 
                       http://eeaax.cmap.polytchnique.fr/
 */
//-----------------------------------------------------------------------------


#ifndef _EOESMUT_H
#define _EOESMUT_H

#include <utils/eoParser.h>
#include <utils/eoRNG.h>
#include <cmath>		// for exp

#include <es/eoESFullChrom.h>
#include <es/eoESInit.h>
#include <eoOp.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

/** ES-style mutation in the large: Obviously, valid only for eoES*

  It is currently valid for three types of ES chromosomes:

  eoEsSimple:   only 1 std deviation
  eoEsStdev:    as many standard deviations as object variables
  eoEsFull:     The whole guacemole: correlations, stdevs and object variables

  Each of these three variant has it's own operator() in eoEsMutate
*/

template <class EOT>
class eoESMutate: public eoMonOp< EOT > {
public:

    typedef EOT::Fitness FitT;

    /** Initialization
        parameters:

        @param _init    proxy class for initializating the three parameters eoEsMutate needs
        @param _bounds  the bounds for the objective variables
    */
    eoESMutate(eoESMutationInit& _init, eoESObjectiveBounds& _bounds) : bounds(_bounds)
    {
        unsigned size = bounds.chromSize();

        if (eoEsInfo<EOT>::single_stdev)
        {
            TauLcl = _init.TauLcl();
            TauLcl /= sqrt((double) size);
        }
        else
        {
            TauLcl = _init.TauLcl();
            TauGlb = _init.TauGlb();

            // renormalization
	        TauLcl /= sqrt( 2.0 * sqrt( (double)size ) );
	        TauGlb /= sqrt( 2.0 * ( (double) size ) );
	    
            if (eoEsInfo<EOT>::has_correlation) 
            { // Correlated Mutations
	            TauBeta = _init.TauBeta();
	        }
        }
    }

    /// needed virtual dtor
    virtual ~eoESMutate() {};
    
  /** Inherited from eoObject 
      @see eoObject
  */
  virtual std::string className() const {return "eoESMutate";};
  
  /**
    Mutate eoEsSimple
  */
  virtual void operator()( eoEsSimple<FitT>& _eo) const
  {
      _eo.stdev *= exp(TauLcl * rng.normal());	

      if (_eo.stdev < eoEsInfo<EOT>::stdev_eps)
	        _eo.stdev = eoEsInfo<EOT>::stdev_eps;
      
      // now apply to all

      for (unsigned i = 0; i < _eo.size(); ++i)
      {
          _eo[i] += _eo.stdev * rng.normal();
      }

      keepInBounds(_eo);
  }
  
  /// mutations - standard and correlated
  //  ========= 
  /*
   *	Standard mutation of object variables and standard 	
   *	deviations in ESs. 
   *	If there are fewer different standard deviations available 
   *	than the dimension of the objective function requires, the 
   * 	last standard deviation is responsible for ALL remaining
   *	object variables.
   *	Schwefel 1977: Numerische Optimierung von Computer-Modellen
   *	mittels der Evolutionsstrategie, pp. 165 ff.
   */

  virtual void operator()( eoESStdev<FitT>& _eo ) const 
  {
    double global = exp(TauGlb * rng.normal());
    for (unsigned i = 0; i < _eo.size(); i++) 
      {
	    double stdev = _eo.stdevs[i];
	    stdev *= global * exp(TauLcl * rng.normal());	

	    if (stdev < eoEsInfo<EOT>::stdev_eps)
	        stdev = eoEsInfo<EOT>::stdev_eps;

	    _eo.stdevs[i] = stdev; 
	    _eo[i] += stdev * rng.normal();
      }

    keepInBounds(_eo);
  }

  /*
   *	Correlated mutations in ESs, according to the following
   *	sources:
   *	H.-P. Schwefel: Internal Report of KFA Juelich, KFA-STE-IB-3/80
   *	p. 43, 1980
   *	G. Rudolph: Globale Optimierung mit parallelen Evolutions-
   *	strategien, Diploma Thesis, University of Dortmund, 1990
   */
  
  // Code from Thomas Baeck 
  
  virtual void operator()( eoEsFull<fitT> & _eo ) const 
  {

    /*
     *	First: mutate standard deviations (as above).
     */
    
    double global = exp(TauGlb * rng.normal());
    for (unsigned i = 0; i < _eo.size(); i++) 
    {
	    double stdev = _eo.stdevs[i];
	    stdev *= global * exp(TauLcl * rng.normal());	

	    if (stdev < eoEsInfo<EOT>::stdev_eps)
	        stdev = eoEsInfo<EOT>::stdev_eps;

	    _eo.stdevs[i] = stdev; 
    }

    
    /*
     *	Mutate rotation angles.
     */
    
    for (i = 0; i < _eo.correlations.size(); i++) 
    {
      _eo.correlations[i] += TauBeta * rng.normal(); 
      if ( fabs(_eo.correlations[i]) > M_PI ) 
      {
	    _eo.correlations[i] -= M_PI * (int) (_eo.correlations[i]/M_PI) ;
      }
    }
    
    /*
     *	Perform correlated mutations.
     */
    unsigned i,k;

    std::vector<double> VarStp(_eo.size());
    for (i = 0; i < _eo.size(); i++) 
      VarStp[i] = _eo.stdevs[i] * rng.normal();

    unsigned nq = _eo.correlations.size() - 1;

    for (k = 0; k < _eo.size()-1; k++) 
    {
      n1 = _eo.size() - k - 1;
      n2 = _eo.size() - 1;
      
      for (i = 0; i < k; i++) 
      {
	    d1 = VarStp[n1];
	    d2 = VarStp[n2];
	    S  = sin( _eo.correlations[nq] );
	    C  = cos( _eo.correlations[nq] );
	    VarStp[n2] = d1 * S + d2 * C;
	    VarStp[n1] = d1 * C - d2 * S;
	    n2--;
	    nq--;
      }
    }
    
    for (i = 0; i < _eo.size(); i++) 
      _eo[i] += VarStp[i];

    keepInBounds(_eo);
  }

  void keepInBounds(EOT& _eo) const
  {
      for (unsigned i = 0; i < _eo.size(); ++i)
      {
          if (_eo[i] < bounds.minimum(i))
              _eo[i] = bounds.minimum(i);
          else if (_eo[i] > bounds.maximum(i))
              _eo[i] = bounds.maximum(i);
      }
  }

  private :
  // the data 
  //=========
  double TauLcl;	/* Local factor for mutation of std deviations */
  double TauGlb;	/* Global factor for mutation of std deviations */
  double TauBeta;	/* Factor for mutation of correlation parameters  */

  eoESObjectiveBounds& bounds;
};

/*
 *	Correlated mutations in ESs, according to the following
 *	sources:
 *	H.-P. Schwefel: Internal Report of KFA Juelich, KFA-STE-IB-3/80
 *	p. 43, 1980
 *	G. Rudolph: Globale Optimierung mit parallelen Evolutions-
 *	strategien, Diploma Thesis, University of Dortmund, 1990
 */

#endif

