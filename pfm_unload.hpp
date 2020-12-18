
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/

#ifndef _PFM_UNLOAD_H_
#define _PFM_UNLOAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <getopt.h>


// Local Includes.

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"

#include "gsf.h"

#include "llz.h"

#include "dted.h"

#include "version.hpp"


class pfm_unload : QObject
{
  Q_OBJECT


public:

  pfm_unload (int32_t argc = 0, char **argv = NULL);
  ~pfm_unload ();


protected:

  void usage ();


  char            out_str[2048];


protected slots:


};

#endif
