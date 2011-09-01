/*
* <peo_debug.cpp>
* Copyright (C) DOLPHIN Project-Team, INRIA Futurs, 2006-2008
* (C) OPAC Team, LIFL, 2002-2008
*
* Sebastien Cahon, Alexandru-Adrian Tantar, Clive Canape
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

#include "peo_debug.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#include "peo_debug.h"

#define MAX_BUFF_SIZE 1000

#define DEBUG_PATH "./log/"


static bool debug = false;

static char host [MAX_BUFF_SIZE];

std :: vector <FILE *> files;


void setDebugMode (bool __dbg)
{

  debug = __dbg;
  gethostname (host, MAX_BUFF_SIZE);
}

extern int getNodeRank ();

void initDebugging ()
{

  mkdir (DEBUG_PATH, S_IRWXU);
  //  files.push_back (stdout);
  char buff [MAX_BUFF_SIZE];
  sprintf (buff, "%s/%d", DEBUG_PATH, getNodeRank ());
  files.push_back (fopen (buff, "w"));
}

void endDebugging ()
{

  for (unsigned i = 0; i < files.size (); i ++)
    if (files [i] != stdout)
      fclose (files [i]);
  files.clear();
}

void printDebugMessage (const char * __mess)
{

  if (debug)
    {

      char buff [MAX_BUFF_SIZE];
      char localTime [MAX_BUFF_SIZE];
      time_t t = time (0);

      /* Date */
      strcpy( localTime, ctime (& t) );
      localTime[ strlen( localTime )-1 ] = ']';
      sprintf (buff, "[%s][%s: ", host, localTime );

      for (unsigned i = 0; i < files.size (); i ++)
        fprintf (files [i],"%s" ,buff);

      /* Message */
      sprintf (buff, "%s", __mess);

      for (unsigned i = 0; i < files.size (); i ++)
        {
          fputs (buff, files [i]);
          fputs ("\n", files [i]);
          fflush (files [i]);
        }
    }
}
