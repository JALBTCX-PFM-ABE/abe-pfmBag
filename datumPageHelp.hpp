
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



QString unitsText =
  datumPage::tr ("Select the unit of measurement for the data with this drop down box.");

QString elevOffText =
  datumPage::tr ("Set a value to be <b>added</b> to each BAG Elevation value.  This may be used for "
                 "datum shifting.  The value will be <b>added</b> to the Elevation after it has been "
                 "converted from a depth.<br><br>"
                 "<b>IMPORTANT NOTE: This value will be ignored if you have selected an optional separation file "
                 "on the start page.</b>");

QString depthCorText =
  datumPage::tr ("Select the sound velocity correction type used for this data.");

QString pfmCrsText = 
  datumPage::tr ("Press this button to select the horizontal Well-known Text (WKT) Coordinate Reference System (CRS) for the input PFM file.  If the input PFM "
                 "file already contains WKT in the header you can still change it... at your own risk.  Please keep in mind that all PFMs built "
                 "with ABE are geographic.  You will not be allowed to select a projected WKT definition.<br><br>"
                 "<b>IMPORTANT NOTE: No vertical transformation will be done!  This button must be used and WKT selected if the button text is <i>None</i>.");

QString pfmWktText = 
  datumPage::tr ("There is already a Well-known Text (WKT) Coordinate Reference System (CRS) defined in the input PFM header.  If the files used to "
                 "create the PFM were LAS files then the PFM WKT is probably correct.  The same goes for CZMIL and/or BAG.  If the PFM was created before "
                 "approximately December 2016 then there is a possibility that the WKT CRS in the PFM header is incorrect.  Please note that <b>all</b> "
                 "PFMs created by ABE are geographic (i.e. latitude and longitude) so you will not be allowed to select a projected WKT CRS.  If you have no "
                 "idea what any of this means you should go ask someone who understands geodesy (which certainly ain't me).<br><br>"
                 "Do you really want to select a different WKT CRS for the input PFM?");

QString bagCrsText = 
  datumPage::tr ("Press this button to select the horizontal Well-known Text (WKT) Coordinate Reference System (CRS) for the output BAG file.  The WKT will determine "
                 "whether the output BAG is geographic or projected.<br><br>"
                 "<b>IMPORTANT NOTE: No vertical transformation will be done!  This button must be used and WKT selected if the button text is <i>None</i>.");

QString vDatumText =
  datumPage::tr ("Select the Vertical Datum for the BAG.  If either <b>WGS84 ELLISPOID</b> or <b>NAVD88 ELLISPOID</b> is selected, the vertical reference system "
                 "will be output to the BAG in Well-known Text (WKT).  You may also input a WKT VERT_CS definition for the vertical coordinate reference system.  "
                 "If any other vertical datum is selected, it will be output as text in the BAG file.<br><br>"
                 "<b>IMPORTANT NOTE</b>You can input your own decription (e.g. NAVD88 geoid12b corrected to MLLW) by selecting the <i>OTHER</i> entry in the "
                 "combo box.  Once you define the text it will appear in the combo box and be output to the BAG as text.</b>");

QString sourceText =
  datumPage::tr ("Enter the source for the data.  Can leave this field blank.");
