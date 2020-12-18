
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



QString surfaceText = 
  surfacePage::tr ("Select the surface type to be used to generate the BAG.<br><br>"
                   "<b>IMPORTANT NOTE: If you select the CUBE Surface you cannot change the bin size.  "
                   "If you need to change the bin size you need to make a new PFM at the correct bin size and run "
                   "pfmCube on that PFM.</b>");

QString uncertaintyText = 
  surfacePage::tr ("Select the data to be used for the uncertainty values.  There are three possible types:<br>"
		   "<ul>"
                   "<li>Standard Deviation - Standard deviation of soundings that contributed to the node</li>"
		   "<li>Average TPE - Average Total Propagated Error from vertical uncertainty of soundings</li>"
		   "<li>Final Uncertainty - Max of CUBE Standard Deviation and Average TPE</li>"
		   "</ul><br><br>"
                   "<b>IMPORTANT NOTE: If you are using pfmFeature points to create the enhanced navigation surface "
                   "and you select Average TPE or Final Uncertainty, the uncertainty in the area of a feature will be "
                   "a combination of the Average TPE/Final Uncertainty and the vertical TPE of the minimum sounding "
                   "in the bin.  The blending of these two values will be done in the same manner as the blending of "
                   "the elevation values for the enhanced surface.  For more information see the What's This help for "
                   "<i>Use features for enhanced surface</i> check box.</b>");

QString featureText = 
  surfacePage::tr ("Selecting this option will cause the associated feature file to be used to create an enhanced navigation surface.  "
                   "The enhanced surface consists of the average surface in areas where there are no significant features (as selected "
                   "by pfmFeature).  In areas where pfmFeature found significant features the surface is a combination of the minimum "
                   "value and the average value with the ratio between the two based on the distance from the feature and the maximum "
                   "distance of the trigger points (fall off of more than IHO Order 1 or Special Order depth criteria) computed in "
                   "pfmFeature.  The algorithm works as follows:<br><br>"
                   "If the BAG node is less than our trigger distance away from any feature, we want to use a combination of the "
                   "minimum depth in the bin and the average depth for the bin.  We use a power of ten (log) curve to blend the two "
                   "depths together.  Linear blending falls off too quickly and leaves you with the same old spike sticking up (like "
                   "we used to have with the tracking list).  The blending works by taking 100 percent of the minimum depth in the bin "
                   "in which the feature is located and 100 percent of the average depth in bins that are more than the trigger "
                   "distance away from the feature.  As we move away from the feature (but still inside the trigger distance) we include "
                   "more of the average and less of the minimum (based on the precomputed log curve).  If the search radii of two features "
                   "overlap we add the minimum depth components (not to exceed 100 percent).  If, at any point in the feature "
                   "comparison for a single bin, we exceed 100 percent we stop doing the feature comparison for that bin.<br><br>"
                   "<b>IMPORTANT NOTE: This option will be disabled if there is no associated feature file.</b>");

QString nonRadiusText =
  surfacePage::tr ("This is the non-pfmFeature feature radius used for blending when making the enhanced surface.  The pfmFeature program "
                   "keeps track of how far it had to search to find the drop off around each feature.  That information is stored in the "
                   "BFD record and is used by pfmBag to decide how far out to go to blend the surface between the minimum and the average. "
                   "If the feature was not made with pfmFeature then you have no idea how far to blend but, hey, I guess this is better "
                   "than a jab in the eye with a sharp stick (although, not much better).");

QString titleText =
  surfacePage::tr ("Enter a title for the BAG.");

QString individualNameText =
  surfacePage::tr ("Enter the name of the person who will be responsible for the BAG.  This is usually a certified hydrographer.");

QString positionNameText =
  surfacePage::tr ("Enter the position title of the person who will be responsible for the BAG.");

QString individualName2Text =
  surfacePage::tr ("Enter the name of the person who is creating the BAG.  This person will be the POC for this BAG.");

QString positionName2Text =
  surfacePage::tr ("Enter the position title of the person who is creating the BAG.");

QString abstractText =
  surfacePage::tr ("Enter any general comments about the data set that is being placed in the BAG.  This will be stored in the "
		   "BAG metadata as the <abstract> field.");

QString mBinSizeText = 
  surfacePage::tr ("Enter the desired BAG bin size in meters.  Normally, this size is chosen based on the <b><i>best resolution of "
                   "the sensor</i></b>.  A good rule of thumb is to take the sonar footprint size or LIDAR spot density at the "
                   "average depth in the area.  Using this bin size generates bins of approximately the requested size in meters.  "
                   "If you set bin size using this field it overrides the geographic bin size.");

QString gBinSizeText = 
  surfacePage::tr ("Enter the desired BAG bin size in decimal minutes of latitude.  Using this bin size generates bins of "
                   "exactly the desired size in minutes of latitude and longitude.  North of 64N and south of 64S the bins "
                   "will be exactly the entered value in latitude but will be the equivalent distance in the longitudinal "
                   "direction at the north or south boundary of the area (depending on hemisphere).  For example, an area "
                   "that extends from 80N to 83N with a bin size setting of 0.1 minutes would have and approximate "
                   "longitudinal bin size of 0.5759 minutes.  If you set bin size using this field it overrides the meter bin "
                   "size.");
