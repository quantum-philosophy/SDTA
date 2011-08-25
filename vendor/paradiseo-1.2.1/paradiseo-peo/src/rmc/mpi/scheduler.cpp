/*
* <scheduler.cpp>
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

#include <queue>

#include "scheduler.h"
#include "tags.h"
#include "mess.h"
#include "../../core/peo_debug.h"

static std :: queue <SCHED_RESOURCE> resources; /* Free resources */

static std :: queue <SCHED_REQUEST> requests; /* Requests */

static unsigned initNumberOfRes = 0;

extern void wakeUpCommunicator();

void initScheduler ()
{

  resources = std :: queue <SCHED_RESOURCE> ();
  requests = std :: queue <SCHED_REQUEST> ();
  initNumberOfRes = 0;

  for (unsigned i = 0; i < the_schema.size (); i ++)
    {

      const Node & node = the_schema [i];

      if (node.rk_sched == my_node -> rk)
        for (unsigned j = 0; j < node.num_workers; j ++)
          resources.push (std :: pair <RANK_ID, WORKER_ID> (i, j + 1));
    }
  initNumberOfRes = resources.size ();
}

bool allResourcesFree ()
{
  return resources.size () == initNumberOfRes;
}

unsigned numResourcesFree ()
{
  return resources.size ();
}

static void update ()
{

  unsigned num_alloc = std :: min (resources.size (), requests.size ());

  for (unsigned i = 0; i < num_alloc; i ++)
    {

      SCHED_REQUEST req = requests.front ();
      requests.pop ();

      SCHED_RESOURCE res = resources.front ();
      resources.pop ();

      printDebugMessage ("allocating a resource.");
      initMessage ();
      pack (req.second);
      pack (res);
      sendMessage (req.first, SCHED_RESULT_TAG);
    }
}

void unpackResourceRequest ()
{

  printDebugMessage ("queuing a resource request.");
  SCHED_REQUEST req;
  unpack (req);
  requests.push (req);
  update ();
}

void unpackTaskDone ()
{

  printDebugMessage ("I'm notified a worker is now idle.");
  SCHED_RESOURCE res;
  unpack (res);
  resources.push (res);
  if (resources.size () == initNumberOfRes)
    printDebugMessage ("all the resources are now free.");
  update ();
  wakeUpCommunicator();
}
