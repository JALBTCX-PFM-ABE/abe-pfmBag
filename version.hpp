
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




#ifndef VERSION

#define     VERSION     "PFM Software - pfmBag V4.22 - 08/07/19"

#endif

/*! <pre>

  Version 1.0
  Jan C. Depner
  06/17/08

  First working version.


  Version 1.01
  Jan C. Depner
  12/04/08

  Added code to correct target depths to nearest PFM_SELECTED_FEATURE depth or nearest valid depth in
  the associated PFM.  We also correct the target XML file to match.


  Version 1.02
  Jan C. Depner
  12/11/08

  Fixed free of depth records if none were read.


  Version 1.03
  Jan C. Depner
  01/29/09

  Set checkpoint to 0 prior to calling open_existing_pfm_file.


  Version 1.10
  Jan C. Depner
  04/05/09

  Removed support for NAVO standard target file (XML) and replaced with Binary Feature Data (BFD) file support.


  Version 1.11
  Jan C. Depner
  04/24/09

  Fixed bug if classification authority was N/A.


  Version 1.12
  Jan C. Depner
  05/21/09

  Set all QFileDialogs to use List mode instead of Detail mode.


  Version 1.13
  Jan C. Depner
  05/26/09

  Fix "writing tracking list" progress bar bug.  Make the weighted surface an option.


  Version 1.14
  Jan C. Depner
  06/24/09

  Changed the BFD names to avoid collision with GNU Binary File Descriptor library.


  Version 1.15
  Jan C. Depner
  08/04/09

  Changed to use OpenNS 1.2 RC1.


  Version 1.16
  Jan C. Depner
  07/09/10

  Use diagonal of bin size when creating weighted surface for "enhanced" BAG surface.  Use minimum depth
  anywhere inside the feature search radius (no longer using blend of min and avg).


  Version 1.17
  Jan C. Depner
  09/17/10

  Now uses the "max dist" information from pfmFeature to determine the radius around each feature to be used for
  the "enhanced" surface.
  Uses a blend of min and max based on a log curve moving away from the shoal point when creating the enhanced surface.
  Distance to end of log curve is defined by the feature radius.


  Version 2.00
  Jan C. Depner
  11/15/10

  Allows user to select a bin size other than the bin size used in the PFM.  This is slower but the flexibility is
  greatly enhanced.  Had to remove options other than the Standard Deviation and T-Quantile from the uncertainty since
  we're not tied to the PFM.


  Version 2.01
  Jan C. Depner
  01/06/11

  Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


  Version 2.02
  Jan C. Depner
  03/18/11

  Fixed bug in standard deviation computation.  Also, added ability to output the average layer to the BAG when we are
  creating an enhanced surface (when it finally gets implemented in BAG).


  Version 3.00
  Jan C. Depner
  03/21/11

  UTM - UTM - UTM!!!  Now produces UTM output.  Is it compatible with SABER?  Who the hell knows.
  Outputs the Average layer when building the enhanced surface.  Unfortunately, I've had to place the Average
  surface in the Nominal_Elevation layer since the Average optional layer has not yet been implemented in BAG.
  Also, removed useless projections from projection combo box.  BAG only supports Geodetic and UTM.


  Version 3.01
  Jan C. Depner
  04/06/11

  Now puts the Average layer in the Average layer (go figure).  I had to patch the BAG library to get it to work.


  Version 3.10
  Jan C. Depner
  04/24/11

  Added ellipsoid/datum separation surface generation from a CHRTR2 datum surface file.  At a meeting with Steve Harrison,
  Wade Ladner, and Bill Rankin, we decided to remove the "Average" surface since it won't significantly affect model
  generation to use the enhanced surface (since most models will use larger bins with averaged enhanced points).


  Version 3.11
  Jan C. Depner
  06/27/11

  Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.


  Version 3.12
  Jan C. Depner
  07/22/11

  Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
  last used directory are in the sidebar URL list of QFileDialogs.


  Version 3.13
  Jan C. Depner
  07/25/11

  Added ability to use ASCII sep file as well as CHRTR2 sep file.


  Version 3.14
  Jan C. Depner
  08/02/11

  Bug fix relating to sep file inclusion.  Segfault if you didn't include the sep file.


  Version 3.15
  Jan C. Depner
  09/21/11

  Replaced compute_index calls with compute_index_ptr calls.


  Version 3.20
  Jan C. Depner
  09/23/11

  Now using BAG 1.4.0 (compressing).


  Version 3.30
  Jan C. Depner
  11/04/11

  Added CUBE Surface to the surface menu.  Added Average TPE to the uncertainty menu.  Blend the
  Average TPE or Final Uncertainty with the TPE fo the minimum sounding in the area of features
  when creating the enhanced navigation surface.


  Version 3.31
  Jan C. Depner
  11/30/11

  Converted .xpm icons to .png icons.


  Version 3.40
  Jan C. Depner
  01/10/12

  Finally got to check the north of 64N loading with geographic bins.  Turns out that we didn't
  need it (and it didn't work properly anyway).  So, I've commented out that code.


  Version 3.41
  Jan C. Depner
  03/08/12

  Final uncertainty and average TPE were switched.


  Version 3.42
  Jan C. Depner
  03/30/12

  Had to turn off compression.  BAGs weren't portable between OSes if it was on.


  Version 3.50
  Jan C. Depner
  04/23/12

  Fixed an error with computing geodetic bin size.  Specifically, the X bin size.  I was computing
  the central X and Y position of the BAG incorrectly.  Thanx and a tip of the hat to Matt Thompson
  for finding the problem.


  Version 3.51
  Jan C. Depner
  05/10/12

  Added elevation offset value to be used for datum shifting when sep surfaces are not available.


  Version 4.00
  Jan C. Depner
  06/07/12

  Changed to corner post (also known as grid) positioning from PFM's center post (also known as pixel)
  positioning of the final output.  In actuality, all we did was to change the final boundaries to be
  one-half cell size smaller.  Et voila, the center posts are now at the corners.


  Version 4.01
  Jan C. Depner (PFM Software)
  12/09/13

  Switched to using .ini file in $HOME (Linux) or $USERPROFILE (Windows) in the ABE.config directory.  Now
  the applications qsettings will not end up in unknown places like ~/.config/navo.navy.mil/blah_blah_blah on
  Linux or, in the registry (shudder) on Windows.


  Version 4.02
  Stacy Johnson
  01/02/14

  Added bag 1.5.2 support.


  Version 4.03
  Stacy Johnson
  03/19/14

  Bag metadata corrections and cleanup.


  Version 4.04
  Jan C. Depner (PFM Software)
  05/07/14

  Fix fprintf statement bug.


  Version 4.05
  Jan C. Depner (PFM Software)
  07/01/14

  - Replaced all of the old, borrowed icons with new, public domain icons.  Mostly from the Tango set
    but a few from flavour-extended and 32pxmania.


  Version 4.06
  Jan C. Depner (PFM Software)
  07/05/14

  - Had to change the argument order in pj_init_plus for the UTM projection.  Newer versions of 
    proj4 appear to be very sensitive to this.


  Version 4.07
  Jan C. Depner (PFM Software)
  07/23/14

  - Switched from using the old NV_INT64 and NV_U_INT32 type definitions to the C99 standard stdint.h and
    inttypes.h sized data types (e.g. int64_t and uint32_t).


  Version 4.08
  Jan C. Depner (PFM Software)
  07/29/14

  - Fixed errors discovered by cppcheck.


  Version 4.09
  Jan C. Depner (PFM Software)
  02/16/15

  - To give better feedback to shelling programs in the case of errors I've added the program name to all
    output to stderr.


  Version 4.10
  Jan C. Depner (PFM Software)
  03/31/15

  - Added ability to use ESRI Polygon, PolygonZ, PolygonM, PolyLine, PolyLineZ, or PolyLineM geographic shape
    files as area files.


  Version 4.11
  Jan C. Depner (PFM Software), Jim Hammack (New Wave Systems)
  04/03/15

  - Computes zone prior to defining UTM projection so that we don't need PROJ_LIB set (I think).


  Version 4.12
  Jan C. Depner (PFM Software)
  06/27/15

  - Fixed PROJ4 init problem.


  Version 4.13
  Jan C. Depner (PFM Software)
  07/23/16

  - Changes made to support BAG 1.5.2 broke all functionality with the exception of generating a WGS84 geodetic BAG.
    I was changing the code to add the pfmFeature points as tracking list points (since it won't really change the 
    bin value and will carry the feature points along with it) when I found the disaster.  What I really discovered
    was that you have to write the metadata for the tracking list before you write the surfaces and the tracking
    list (apparently).  It's also really nice to not allocate memory for the same pointer more than once ;-)
  - Fixed "free" error when not building enhanced, weighted surface.
  - Fixed multiple allocation of data space in the BAG metadata.
  - Compute sizes of BAG metadata u8 fields instead of using arbitrary size.
  - Removed hard-wired spatialReference setting to geodetic.
  - Use WKT if it is present in the PFM header (only for WGS84 or NAD83).


  Version 4.14
  Jan C. Depner (PFM Software)
  08/19/16

  - Now supports BAG 1.5.3
  - Added Elevation_Solution_Group surface.  Each node contains the shoal elevation, standard
    deviation, and number of contributing soundings for each bin.
  - Added Node_Group surface when using the CUBE surface.  Each node contains the number of
    CUBE hypotheses and the CUBE hypothesis strength.
  - Flipped Z0 and Z1 on the optional corrector surface to match SABER's terminology.
  - Removed ability to exclude the uncertainty layer.
  - Added data compression to make up for the extra layers.  The files are now smaller than they
    were without the extra layers.


  Version 4.15
  Jan C. Depner (PFM Software)
  08/20/16

  - Added support for Binary Feature Data (BFD) 3.0 feature type field.  In other words,
    BFDATA_INFORMATIONAL type features are ignored.
  - Added default radius to use when making the enhanced surface and you have non-pfmFeature
    features.


  Version 4.16
  Jan C. Depner (PFM Software)
  08/21/16

  - Now sets list_series correctly and keeps track of parent/child relationship in the lineageProcessSteps
    entry for each tracking list item.


  Version 4.17
  Jan C. Depner (PFM Software)
  08/24/16

  - Horizontal and vertical datums are now selectable between WGS84 and NAD83.


  Version 4.18
  Jan C. Depner (PFM Software)
  08/27/16

  - Now uses the same font as all other ABE GUI apps.  Font can only be changed in pfmView Preferences.


  Version 4.19
  Jan C. Depner (PFM Software)
  08/29/16

  - Fixed problem trying to use PFM WKT when writing UTM BAG.
  - Fixed out of bounds condition when using CUBE surface to write UTM BAG.
  - Removed unused fields in wizard pages.


  Version 4.20
  Jan C. Depner (PFM Software)
  10/13/16

  - Added ability to input Well-known Text (WKT) Coordinate Reference System (CRS) definition for PFM (if it's not already
    there) and BAG.  This uses the "Recent WKT" strings in globalABE.ini that are shared with pfmLoader, trackLine, and
    fileEdit3D.  Hopefully, if I did it right, this will allow you to transform from one geographic system to another in
    addition to transforming from geographic to projected systems.  This stuff is incredibly confusing!
  - Fixed the fact that I wasn't adjusting Z.  Proj.4 is supposed to do the Z transform as well as the X and Y so I added
    it to the final pj_transform call.


  Version 4.21
  Jan C. Depner (PFM Software)
  10/24/17

  - A bunch of changes to support doing translations in the future.  There is a generic
    pfmBag_xx.ts file that can be run through Qt's "linguist" to translate to another language.


  Version 4.22
  Jan C. Depner (PFM Software)
  08/07/19

  - Now that get_area_mbr supports shape files we don't need to handle it differently from the other
    area file types.

</pre>*/
