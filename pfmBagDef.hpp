
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


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef PFMBAGDEF_H
#define PFMBAGDEF_H

#include <QtCore>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <stdtypes.h>
#include <proj_api.h>

#include "nvutility.h"
#include "nvutility.hpp"

#include "pfm.h"
#include "hyp.h"
#include "hdf5.h"

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>

#include "bag.h"
#include "bag_legacy.h"

#include "binaryFeatureData.h"

#include "chrtr2.h"
#include "shapefil.h"


#define MIN_SURFACE  0
#define MAX_SURFACE  1
#define AVG_SURFACE  2
#define CUBE_SURFACE 3

#define STD_UNCERT   0
#define TPE_UNCERT   1
#define FIN_UNCERT   2


typedef struct
{
  uint8_t            active;
  QString            abbrev;
  QString            name;
} DATUM;


typedef struct
{
  QString       pfm_file_name;
  QString       avg_surface_name;
  int32_t       window_x;
  int32_t       window_y;
  int32_t       window_width;
  int32_t       window_height;
  int32_t       surface;
  double        mbin_size;
  double        gbin_size;
  int32_t       uncertainty;
  uint8_t       enhanced;
  int32_t       units;                 //  0 - meters, 1 - feet, 2 - fathoms, 3 - cubits, 4 - willetts
  int32_t       depth_cor;             //  0 - corrected, 1 - 1550 m/s, 2 - 4800 ft/s, 3 - 800 fm/s, 4 - mixed
  DATUM         v_datums[100];         //  From icons/vertical_datums.txt
  int32_t       v_datum_count;
  int32_t       v_datum;
  QString       wktString[10];         //  QStrings holding recently used WKT settings
  QString       pfm_wkt;
  QString       bag_wkt;
  float         elev_off;              //  Offset (in "units") to be ADDED to each BAG Elevation
  float         non_radius;            //  Radius to be used for non-pfmFeature features when making the enhanced surface.
  QString       source;
  QString       comments;
  int32_t       classification;        //  0 - Unclassified, 1 - Confidential, 2 - Secret, 3 - Top Secret
  int32_t       authority;
  QDate         declassDate;
  QDate         compDate;
  QString       distStatement;
  QString       title;
  QString       pi_name;
  QString       pi_title;
  QString       poc_name;
  QString       abstract;
  QString       input_dir;
  QString       output_dir;
  QString       area_dir;
  QString       sep_dir;
  char          progname[256];
  QFont         font;                  //  Font used for all ABE GUI applications
} OPTIONS;



typedef struct
{
  QGroupBox           *wbox;
  QGroupBox           *mbox;
  QGroupBox           *gbox;
  QProgressBar        *wbar;
  QProgressBar        *mbar;
  QProgressBar        *gbar;
} RUN_PROGRESS;



#endif
