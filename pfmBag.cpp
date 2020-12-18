
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



#include "pfmBag.hpp"
#include "pfmBagHelp.hpp"



double settings_version = 1.0;


pfmBag::pfmBag (int32_t *argc, char **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  strcpy (options.progname, argv[0]);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfmBagWatermark.png"));


  /*  Override the HDF5 version check so that we can read BAGs created with an older version of HDF5.  */

  putenv ((char *) "HDF5_DISABLE_VERSION_CHECK=2");


  //  Read in the vertical datum information.

  options.v_datum_count = 0;

  char string[256];
  QString qstring;


  QFile *vDataFile = new QFile (":/icons/vertical_datums.txt");

  if (vDataFile->open (QIODevice::ReadOnly))
    {
      while (vDataFile->readLine (string, sizeof (string)) > 0)
        {
          qstring = QString (string);
          options.v_datums[options.v_datum_count].active = (uint8_t) (qstring.section (':', 0, 0).toInt ());
          options.v_datums[options.v_datum_count].abbrev = qstring.section (':', 1, 1);
          options.v_datums[options.v_datum_count].name = qstring.section (':', 2, 2).simplified ();

          options.v_datum_count++;
        }
      vDataFile->close ();
    }
  else
    {
      qstring.sprintf (tr ("%s %s %s %d - Can't open the vertical datum file").toLatin1 (), options.progname, __FILE__, __FUNCTION__, __LINE__);
      QMessageBox::critical (this, "pfmBag", qstring);
      exit (-1);
    }


  //  Get the user's defaults if available

  envin (&options);


  // Set the application font

  QApplication::setFont (options.font);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, true);
  setOption (ExtendedWatermarkPixmap, false);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  area_file_name = tr ("NONE");


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  setPage (0, new startPage (argc, argv, this, &options));

  setPage (1, sPage = new surfacePage (this, &options));

  setPage (2, dPage = new datumPage (this, &options));

  setPage (3, new classPage (this, &options));

  setPage (4, new runPage (this, &progress, &checkList));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, true);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start generating the BAG"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);


  if (getenv ("BAG_HOME") == NULL)
    {
      QMessageBox::information (this, tr ("pfmBag Error"), tr ("BAG_HOME environment variable is not set.\n"
                                                               "This must point to the configdata directory or pfmBag will fail."), QMessageBox::Ok,  QMessageBox::Ok);

      exit (-1);
    }
}



pfmBag::~pfmBag ()
{
}



void pfmBag::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.png"));
  button (QWizard::CustomButton1)->setEnabled (false);


  switch (id)
    {
    case 0:    //  Start page
      break;

    case 1:    //  Surface page

      options.pfm_file_name = field ("pfm_file_edit").toString ();
      sPage->setFields (&options);
      break;

    case 2:    //  Datum page

      dPage->setFields (&options);
      break;

    case 3:    //  Classification page

      break;

    case 4:    //  Run page

      button (QWizard::CustomButton1)->setEnabled (true);

      area_file_name = field ("area_file_edit").toString ();
      sep_file_name = field ("sep_file_edit").toString ();
      output_file_name = field ("output_file_edit").toString ();


      //  Save the output directory.  It might have been input manually instead of browsed.

      options.output_dir = QFileInfo (output_file_name).absoluteDir ().absolutePath ();

      options.uncertainty = field ("uncertainty").toInt ();

      options.units = field ("units").toInt ();

      options.elev_off = field ("elevOff").toFloat ();
      if (!sep_file_name.isEmpty ()) options.elev_off = 0.0;

      options.depth_cor = field ("depthCor").toInt ();

      options.source = field ("source").toString ();

      options.classification = field ("classification").toInt ();
      options.authority = field ("authority").toInt ();
      options.declassDate = field ("declassDate").toDate ();
      options.compDate = field ("compDate").toDate ();

      options.distStatement = field ("distStatement").toString ();

      options.non_radius = field ("nonRadius").toDouble ();
      options.title = field ("title").toString ();
      options.pi_name = field ("individualName").toString ();
      options.pi_title = field ("positionName").toString ();
      options.poc_name = field ("individualName2").toString ();
      options.abstract = field ("abstract").toString ();


      //  Can't make an enhanced surface using minutes.

      if (options.mbin_size == 0.0) options.enhanced = NVFalse;


      if (!options.enhanced)
        {
          progress.wbar->hide ();
          progress.wbox->hide ();
        }
      else
        {
          progress.wbar->show ();
          progress.wbox->show ();
        }


      //  Got to have a title.

      if (options.title.isEmpty ()) options.title = QFileInfo (output_file_name).baseName ();


      //  Got to have an abstract.

      if (options.abstract.isEmpty ()) options.abstract = QFileInfo (output_file_name).baseName ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      //  Save the options.

      envout (&options);


      QString string;

      checkList->clear ();

      string = tr ("Input PFM file : %1").arg (options.pfm_file_name);
      checkList->addItem (string);

      string = tr ("Output file(s) : %1").arg (output_file_name);
      checkList->addItem (string);

      if (!area_file_name.isEmpty ())
        {
          string = tr ("Area file : %1").arg (area_file_name);
          checkList->addItem (string);
        }

      if (!sep_file_name.isEmpty ())
        {
          string = tr ("Separation file : %1").arg (sep_file_name);
          checkList->addItem (string);
        }

      switch (options.surface)
        {
        case MIN_SURFACE:
          string = tr ("Minimum Filtered Surface");
          checkList->addItem (string);
          break;

        case MAX_SURFACE:
          string = tr ("Maximum Filtered Surface");
          checkList->addItem (string);
          break;

        case AVG_SURFACE: 
          string = tr ("Average Filtered Surface");
          checkList->addItem (string);
          break;

        case CUBE_SURFACE: 
          string = tr ("CUBE Surface");
          checkList->addItem (string);
          break;
        }


      switch (options.units)
        {
        case 0:
          string = tr ("Units : meters");
          checkList->addItem (string);
          break;

        case 1:
          string = tr ("Units : feet");
          checkList->addItem (string);
          break;

        case 2:
          string = tr ("Units : fathoms");
          checkList->addItem (string);
          break;
        }


      switch (options.depth_cor)
        {
        case 0:
          string = tr ("Using corrected depth");
          checkList->addItem (string);
          break;

        case 1:
          string = tr ("Using uncorrected Depth @ 1500 m/s");
          checkList->addItem (string);
          break;

        case 2:
          string = tr ("Using uncorrected Depth @ 4800 ft/s");
          checkList->addItem (string);
          break;

        case 3:
          string = tr ("Using uncorrected Depth @ 800 fm/s");
          checkList->addItem (string);
          break;

        case 4:
          string = tr ("Using mixed corrections");
          checkList->addItem (string);
          break;
        }


      switch (options.uncertainty)
        {
        case STD_UNCERT:
          string = tr ("Using Standard Deviation for uncertainty values");
          break;

        case TPE_UNCERT:
          string = tr ("Using average TPE for uncertainty values");
          break;

        case FIN_UNCERT:
          string = tr ("Using Max of CUBE Standard Deviation and average TPE for uncertainty values");
          break;
        }
      checkList->addItem (string);


      if (options.enhanced)
        {
          string = tr ("Using feature points to create enhanced navigation surface");
          checkList->addItem (string);
        }


      string = tr ("Data source : %1").arg (options.source);
      checkList->addItem (string);

      if (options.authority)
        {
          switch (options.classification)
            {
            case 0:
              string = tr ("Classification : Unclassified");
              checkList->addItem (string);
              break;

            case 1:
              string = tr ("Classification : Confidential");
              checkList->addItem (string);
              break;

            case 2:
              string = tr ("Classification : Secret");
              checkList->addItem (string);
              break;

            case 3:
              string = tr ("Classification : Top Secret");
              checkList->addItem (string);
              break;
            }

          switch (options.authority)
            {
            case 0:
              string = tr ("Classifying Authority : N/A");
              checkList->addItem (string);
              break;

            case 1:
              string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(23)");
              checkList->addItem (string);
              break;

            case 2:
              string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(24)");
              checkList->addItem (string);
              break;

            case 3:
              string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(27)");
              checkList->addItem (string);
              break;

            case 4:
              string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(28)");
              checkList->addItem (string);
              break;
            }


          string = tr ("Declassification date : ") + options.declassDate.toString ("yyyy-MM-dd");
          checkList->addItem (string);


          string = tr ("Distribution statement : ") + options.distStatement;
          checkList->addItem (string);
        }


      string = tr ("Extract/Compile date : ") + options.compDate.toString ("yyyy-MM-dd");
      checkList->addItem (string);

      string = tr ("BAG Title : ") + options.title;
      checkList->addItem (string);

      string = tr ("Certifying official's name : ") + options.pi_name;
      checkList->addItem (string);

      string = tr ("Certifying official's position : ") + options.pi_title;
      checkList->addItem (string);

      string = tr ("POC name : ") + options.poc_name;
      checkList->addItem (string);

      string = tr ("BAG comments : %1").arg (options.abstract);
      QListWidgetItem *cur = new QListWidgetItem (string);
      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);

      break;
    }
}



void pfmBag::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;

    case 3:
      break;
    }
}



void pfmBag::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  This is where the fun stuff happens.

void 
pfmBag::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  PFM_OPEN_ARGS                open_args;
  QString                      string;
  int32_t                      pfm_handle, sep_handle = -1, bag_width, bag_height;
  FILE                         *sep_fp = NULL;
  char                         area_file[512], sep_file[512], sep_string[128];
  u8                           name[512];
  uint8_t                      **weight = NULL;
  NV_F64_XYMBR                 proj_mbr = {0.0, 0.0, 0.0, 0.0};
  CHRTR2_HEADER                sep_header;
  CHRTR2_RECORD                sep_record;
  int32_t                      pj_status = 0;
  bagError                     stat;
  BAG_METADATA                 bag_metadata;        
  bagLegacyReferenceSystem     system;
  int32_t                      bfd_handle = -1;
  BFDATA_HEADER                bfd_header;
  BFDATA_SHORT_FEATURE         *feature;
  uint8_t                      features = NVFalse;
  double                       *radius = NULL, log_array[100], lat = 0.0, lon = 0.0, northing = 0.0, easting = 0.0;
  NV_F64_COORD2                xy[2] = {{0.0, 0.0}, {0.0, 0.0}};


  strcpy (open_args.list_path, options.pfm_file_name.toLatin1 ());


  open_args.checkpoint = 0;
  pfm_handle = open_existing_pfm_file (&open_args);


  if (pfm_handle < 0) pfm_error_exit (pfm_error);


  if (strcmp (open_args.target_path, "NONE"))
    {
      if ((bfd_handle = binaryFeatureData_open_file (open_args.target_path, &bfd_header, BFDATA_READONLY)) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::warning (this, "pfmBag", tr ("Unable to open feature file\nReason: %1").arg (msg));
          features = NVFalse;
        }
      else
        {
          binaryFeatureData_read_all_short_features (bfd_handle, &feature);
          features = NVTrue;
        }
    }


  //  Initialize the bag_metadata structure

  stat = bagInitMetadata (&bag_metadata);
  if (stat != BAG_SUCCESS)
    { 
      string = tr ("Error initializing metadata");

      u8 *errstr;

      if (bagGetErrorString (stat, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

      QMessageBox::warning (this, tr ("pfmBag Error"), string);

      exit (-1);
    }


  bag_metadata.fileIdentifier = (u8 *) malloc (sizeof (u8) * 5);
  strcpy ((char *) bag_metadata.fileIdentifier, "test");
  bag_metadata.language = (u8 *) malloc (sizeof (u8) * 5);
  strcpy ((char *) bag_metadata.language, "en");

  QDate current_date = QDate::currentDate ();

  QString date_string = current_date.toString ("yyyy-MM-dd");

  bag_metadata.dateStamp = (u8 *) malloc (sizeof (u8) * date_string.size () + 1);
  strcpy ((char *) bag_metadata.dateStamp , date_string.toLatin1 ());


  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (false);
  button (QWizard::BackButton)->setEnabled (false);
  button (QWizard::CustomButton1)->setEnabled (false);


  //  Move the data into the BAG metadata identificationInfo Structure

  bag_metadata.identificationInfo->title = (u8 *) malloc (sizeof (u8) * options.title.size () + 1);
  strcpy ((char*) bag_metadata.identificationInfo->title, options.title.toLatin1 ());

  bag_metadata.identificationInfo->date = (u8 *) malloc (sizeof (u8) * date_string.size () + 1);
  strcpy ((char *) bag_metadata.identificationInfo->date, date_string.toLatin1 ());

  bag_metadata.identificationInfo->dateType = (u8 *) malloc (sizeof (u8) * 12);
  strcpy ((char *) bag_metadata.identificationInfo->dateType, tr ("publication").toLatin1 ());

  bag_metadata.identificationInfo->numberOfResponsibleParties = 1;
  bag_metadata.identificationInfo->responsibleParties = (BAG_RESPONSIBLE_PARTY *) malloc (sizeof (BAG_RESPONSIBLE_PARTY));

  bag_metadata.identificationInfo->responsibleParties[0].individualName = (u8 *) malloc (sizeof (u8) * options.pi_name.size () + 1);
  strcpy ((char *) bag_metadata.identificationInfo->responsibleParties[0].individualName, options.pi_name.toLatin1 ());

  bag_metadata.identificationInfo->responsibleParties[0].positionName = (u8 *) malloc (sizeof (u8) * options.pi_title.size () + 1);
  strcpy ((char *) bag_metadata.identificationInfo->responsibleParties[0].positionName, options.pi_title.toLatin1 ());

  bag_metadata.identificationInfo->responsibleParties[0].organisationName = (u8 *) malloc (sizeof (u8) * 27);
  strcpy ((char *) bag_metadata.identificationInfo->responsibleParties[0].organisationName, tr ("Naval Oceanographic Office").toLatin1 ());

  bag_metadata.identificationInfo->responsibleParties[0].role = (u8 *) malloc (sizeof (u8) * 23);
  strcpy ((char *) bag_metadata.identificationInfo->responsibleParties[0].role, tr ("Principal investigator").toLatin1 ());

  bag_metadata.identificationInfo->abstractString = (u8 *) malloc (sizeof (u8) * options.abstract.size () + 1);
  strcpy ((char *) bag_metadata.identificationInfo->abstractString, options.abstract.toLatin1 ());

  bag_metadata.identificationInfo->status = (u8 *) malloc (sizeof (u8) * 9);
  strcpy ((char *) bag_metadata.identificationInfo->status, "Complete");

  bag_metadata.identificationInfo->language = (u8 *) malloc (sizeof (u8) * 3);
  strcpy ((char *) bag_metadata.identificationInfo->language, "en");

  bag_metadata.identificationInfo->topicCategory = (u8 *) malloc (sizeof (u8) * 10);
  strcpy ((char *) bag_metadata.identificationInfo->topicCategory, "elevation");

  bag_metadata.identificationInfo->spatialRepresentationType = (u8 *) malloc (sizeof (u8) * 5);
  strcpy ((char *) bag_metadata.identificationInfo->spatialRepresentationType, "grid");

  bag_metadata.identificationInfo->nodeGroupType = (u8 *) malloc (sizeof (u8) * 8);
  strcpy ((char *) bag_metadata.identificationInfo->nodeGroupType, "unknown");

  bag_metadata.identificationInfo->elevationSolutionGroupType = (u8 *) malloc (sizeof (u8) * 8);
  strcpy ((char *) bag_metadata.identificationInfo->elevationSolutionGroupType, "unknown");


  NV_F64_XYMBR mbr = open_args.head.mbr;


  if (!area_file_name.isEmpty ())
    {
      double polygon_x[200], polygon_y[200];
      int32_t polygon_count;

      strcpy (area_file, area_file_name.toLatin1 ());

      get_area_mbr (area_file, &polygon_count, polygon_x, polygon_y, &mbr);

      if (mbr.min_y > open_args.head.mbr.max_y || mbr.max_y < open_args.head.mbr.min_y ||
          mbr.min_x > open_args.head.mbr.max_x || mbr.max_x < open_args.head.mbr.min_x)
        {
          QString qstring = QString (tr ("Specified area is completely outside of the PFM bounds!"));
          QMessageBox::critical (this, "pfmBag", qstring);
          exit (-1);
        }
    }


  //  If mbin_size is 0.0 then we're defining bin sizes in minutes of lat/lon

  double x_bin_size_degrees = 0.0;
  double y_bin_size_degrees = 0.0;

  if (options.mbin_size == 0.0)
    {
      y_bin_size_degrees = options.gbin_size / 60.0;
      x_bin_size_degrees = y_bin_size_degrees;


      /*  We've changed our collective minds.  After actually editing some data north of 64N we have found
          that there is no distortion due to elongated bins.  Therefore, Paul Marin has decided (and I agree)
          that we don't need to change the X bin size to match the Y bin size in distance.  I'm leaving the
          code here for reference (it didn't work though because we should have set a computed XY bin size).
          JCD  01/10/12


      //  We're going to use approximately spatially equivalent geographic bin sizes north or 64N and south of 64S.
      //  Otherwise we're going to use equal lat and lon bin sizes.

      if (mbr.min_y >= 64.0 || mbr.max_y <= -64.0)
        {
          double dist, az, y, x;
          if (mbr.min_y <= -64.0)
            {
              invgp (NV_A0, NV_B0, mbr.max_y, mbr.min_x, mbr.max_y - (options.gbin_size / 60.0), mbr.min_x, &dist, &az);
            }
          else
            {
              invgp (NV_A0, NV_B0, mbr.min_y, mbr.min_x, mbr.min_y + (options.gbin_size / 60.0), mbr.min_x, &dist, &az);
            }

          newgp (mbr.min_y, mbr.min_x, 90.0, dist, &y, &x);

          x_bin_size_degrees = x - mbr.min_x;
        }
      */
    }
  else
    {
      NV_F64_COORD2 central, xy;

      central.x = mbr.min_x + (mbr.max_x - mbr.min_x) / 2.0;
      central.y = mbr.min_y + (mbr.max_y - mbr.min_y) / 2.0;


      //  Convert from meters.

      newgp (central.y, central.x, 90.0, options.mbin_size, &xy.y, &xy.x);


      //  Check if the longitude is in the form 0 to 360.

      if (central.x > 180) xy.x = xy.x + 360;

      x_bin_size_degrees = xy.x - central.x;
      newgp (central.y, central.x, 0.0, options.mbin_size, &xy.y, &xy.x);
      y_bin_size_degrees = xy.y - central.y;
    }


  bag_height = NINT ((mbr.max_y - mbr.min_y) / y_bin_size_degrees + 0.05);
  bag_width = NINT ((mbr.max_x - mbr.min_x) / x_bin_size_degrees + 0.05);


  //  Redefine upper and right bounds

  mbr.max_x = mbr.min_x + bag_width * x_bin_size_degrees;
  mbr.max_y = mbr.min_y + bag_height * y_bin_size_degrees;


  //  BAG metadata spatialRepresentationInfo

  bag_metadata.spatialRepresentationInfo->resolutionUnit = (u8 *) malloc (sizeof (u8) * 12);
  if (options.bag_wkt.contains ("PROJCS"))
    {
      strcpy ((char *) bag_metadata.spatialRepresentationInfo->resolutionUnit, "meters");
    }
  else
    {
      strcpy ((char *) bag_metadata.spatialRepresentationInfo->resolutionUnit, "degrees");
    }

  bag_metadata.spatialRepresentationInfo->transformationParameterAvailability = False;

  bag_metadata.spatialRepresentationInfo->cellGeometry = (u8 *) malloc (sizeof (u8) * 6);
  strcpy ((char *) bag_metadata.spatialRepresentationInfo->cellGeometry, "point");  

  bag_metadata.spatialRepresentationInfo->transformationParameterAvailability = False;
  bag_metadata.spatialRepresentationInfo->checkPointAvailability = False;


  bag_metadata.identificationInfo->depthCorrectionType = (u8 *) malloc (sizeof (u8) * 30);

  switch (options.depth_cor)
    {
    case 0:
      strcpy ((char *) bag_metadata.identificationInfo->depthCorrectionType, tr ("Corrected depth").toLatin1 ());
      break;

    case 1:
      strcpy ((char *) bag_metadata.identificationInfo->depthCorrectionType, tr ("Uncorrected 1500 m/s").toLatin1 ());
      break;

    case 2:
      strcpy ((char *) bag_metadata.identificationInfo->depthCorrectionType, tr ("Uncorrected 4800 ft/s").toLatin1 ());
      break;

    case 3:
      strcpy ((char *) bag_metadata.identificationInfo->depthCorrectionType, tr ("Uncorrected 800 fm/s").toLatin1 ());
      break;

    case 4:
      strcpy ((char *) bag_metadata.identificationInfo->depthCorrectionType, tr ("Mixed corrections").toLatin1 ());
      break;
    }


  bag_metadata.identificationInfo->verticalUncertaintyType = (u8 *) malloc (sizeof (u8) * 20);

  switch (options.uncertainty)
    {
    case STD_UNCERT:
      strcpy ((char *) bag_metadata.identificationInfo->verticalUncertaintyType, tr ("Std Dev").toLatin1 ());
      break;

    case TPE_UNCERT:
      strcpy ((char *) bag_metadata.identificationInfo->verticalUncertaintyType, tr ("TPE").toLatin1 ());
      break;

    case FIN_UNCERT:
      strcpy ((char *) bag_metadata.identificationInfo->verticalUncertaintyType, tr ("Final uncertainty").toLatin1 ());
      break;
    }


  //  BAG metadata legalConstraints

  bag_metadata.legalConstraints->otherConstraints = (u8 *) malloc (sizeof (u8) * 5);
  strcpy ((char *) bag_metadata.legalConstraints->otherConstraints, " ");

  bag_metadata.legalConstraints->useConstraints = (u8 *) malloc (sizeof (u8) * 5);
  strcpy ((char *) bag_metadata.legalConstraints->useConstraints, " ");


  //  BAG metadata securityConstraints

  bag_metadata.securityConstraints->classification = (u8 *) malloc (sizeof (u8) * 14);

  switch (options.classification)
    {
    case 0:
      strcpy ((char *) bag_metadata.securityConstraints->classification, tr ("Unclassified").toLatin1 ());
      break;

    case 1:
      strcpy ((char *) bag_metadata.securityConstraints->classification, tr ("Confidential").toLatin1 ());
      break;

    case 2:
      strcpy ((char *) bag_metadata.securityConstraints->classification, tr ("Secret").toLatin1 ());
      break;

    case 3:
      strcpy ((char *) bag_metadata.securityConstraints->classification, tr ("Top Secret").toLatin1 ());
      break;
    }

  switch (options.authority)
    {
    case 0:
      string = tr ("Classifying Authority : N/A");
      break;

    case 1:
      string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(23)\n");
      break;

    case 2:
      string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(24)\n");
      break;

    case 3:
      string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(27)\n");
      break;

    case 4:
      string = tr ("Classifying Authority : Derived from: OPNAVINSTS5513.5B(28)\n");
      break;
    }

  string += tr ("Declassification date : %1\n").arg(options.declassDate.toString ("yyyy-MM-dd"));

  string += tr ("Distribution statement : %1").arg (options.distStatement);

  bag_metadata.securityConstraints->userNote = (u8 *) malloc (sizeof (u8) * string.size () + 1);
  strcpy ((char *) bag_metadata.securityConstraints->userNote, string.toLatin1 ());


  strcpy ((char *) bag_metadata.dataQualityInfo->scope, tr ("dataset").toLatin1 ());

  double half_x = 0.0, half_y = 0.0;


  //  BAG metadata horizontalReferenceSystem

  bag_metadata.horizontalReferenceSystem->definition = (u8 *) malloc (sizeof (u8) * 1024);
  bag_metadata.horizontalReferenceSystem->type = (u8 *) malloc (sizeof (u8) * 10);


  //  BAG metadata verticalReferenceSystem

  bag_metadata.verticalReferenceSystem->definition = (u8 *) malloc (sizeof (u8) * 1024);
  bag_metadata.verticalReferenceSystem->type = (u8 *) malloc (sizeof (u8) * 10);


  //  Make the PFM WKT human readable and set up the proj4 projection.

  OGRSpatialReference pfmSRS;
  char wkt[8192], pretty[8192], proj4[256];
  strcpy (wkt, options.pfm_wkt.toLatin1 ());
  char *ppszPretty, *ppszProj4, *ptr_wkt = wkt;

  pfmSRS.importFromWkt (&ptr_wkt);

  pfmSRS.exportToPrettyWkt (&ppszPretty);
  pfmSRS.exportToProj4 (&ppszProj4);

  strcpy (proj4, ppszProj4);
  OGRFree (ppszProj4);


  if (!(pfm_proj = pj_init_plus (proj4)))
    {
      QMessageBox::critical (this, "pfmBag", tr ("Error initializing input PFM projection"));
      exit (-1);
    }


  strcpy (pretty, ppszPretty);
  OGRFree (ppszPretty);

  string = tr ("PFM WKT : \n%1").arg (QString (pretty));
  checkList->addItem (string);


  //  Make the BAG WKT human readable and setup the output BAG proj4 projection.

  OGRSpatialReference bagSRS;
  strcpy (wkt, options.bag_wkt.toLatin1 ());
  ptr_wkt = wkt;

  bagSRS.importFromWkt (&ptr_wkt);

  bagSRS.exportToPrettyWkt (&ppszPretty);
  bagSRS.exportToProj4 (&ppszProj4);

  strcpy (proj4, ppszProj4);
  OGRFree (ppszProj4);


  if (!(bag_proj = pj_init_plus (proj4)))
    {
      QMessageBox::critical (this, "pfmBag", tr ("Error initializing output BAG projection"));
      exit (-1);
    }


  strcpy (pretty, ppszPretty);
  OGRFree (ppszPretty);

  string = tr ("BAG WKT : \n%1").arg (QString (pretty));
  checkList->addItem (string);


  //  Check to see if the PFM and BAG CRS are the same.

  io_crs_equal = NVFalse;
  if (options.pfm_wkt == options.bag_wkt) io_crs_equal = NVTrue;


  char hbuffer[1024];
  char vbuffer[1024];

  strcpy (hbuffer, options.bag_wkt.toLatin1 ());

  strcpy ((char *) bag_metadata.horizontalReferenceSystem->definition, hbuffer);
  strcpy ((char *) bag_metadata.horizontalReferenceSystem->type, "WKT");


  //  Now set the vertical reference

  strcpy ((char *) bag_metadata.verticalReferenceSystem->type, "WKT");
  if (options.v_datum == 53)
    {
      strcpy (vbuffer, "VERT_CS[\"WGS84E Z in meters\",VERT_DATUM[\"Ellipsoid\",2002],UNIT[\"metre\",1],AXIS[\"Z\",UP]]");
    }
  else if (options.v_datum == 54)
    {
      strcpy (vbuffer, "VERT_CS[\"NAVD88\",VERT_DATUM[\"North American Vertical Datum 1988\",2005,AUTHORITY[\"EPSG\",\"5103\"]],AXIS[\"Gravity-related height\",UP],UNIT[\"metre\",1.0,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"5703\"]]");
    }
  else
    {
      strcpy (vbuffer, options.v_datums[options.v_datum].name.toLatin1 ());
      strcpy ((char *) bag_metadata.verticalReferenceSystem->type, "TEXT");


      //  Check to see if the user put a VERT_CS WKT string into the "OTHER" option...

      if (options.v_datums[options.v_datum].name.startsWith ("VERT_CS"))
        {
          OGRSpatialReference vertSRS;
          char *ptr_wkt = vbuffer;

          if (vertSRS.importFromWkt (&ptr_wkt) == OGRERR_NONE) strcpy ((char *) bag_metadata.verticalReferenceSystem->type, "WKT");
        }
    }
  strcpy ((char *) bag_metadata.verticalReferenceSystem->definition, vbuffer);

  string = tr ("BAG Vertical Datum : \n%1").arg (QString (vbuffer));
  QListWidgetItem *cur = new QListWidgetItem (string);
  checkList->addItem (cur);
  checkList->setCurrentItem (cur);
  checkList->scrollToItem (cur);


  //  If we're doing UTM output, set up the area in northings and eastings.

  if (options.bag_wkt.contains ("PROJCS"))
    {
      system.coordSys = UTM;


      //  Get the min and max northings and eastings.  We're still going to need the actual lat/lon MBR so we store these in proj_mbr.
      //  Note that we're trying to get the largest extents available.  This is mostly due to distortion in longitude.

      double llx, ulx, lrx, urx, lly, uly, lry, ury;

      llx = mbr.min_x * NV_DEG_TO_RAD;
      lly = mbr.min_y * NV_DEG_TO_RAD;
      pj_status = pj_transform (pfm_proj, bag_proj, 1, 1, &llx, &lly, NULL);
      if (pj_status)
        {
          QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
            (pj_strerrno (pj_status)).arg (mbr.min_x, 0, 'f', 11).arg (mbr.min_y, 0, 'f', 11).arg (llx, 0, 'f', 11).arg (lly, 0, 'f', 11);
          QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
          exit (-1);
        }

      ulx = mbr.min_x * NV_DEG_TO_RAD;
      uly = mbr.max_y * NV_DEG_TO_RAD;
      pj_status = pj_transform (pfm_proj, bag_proj, 1, 1, &ulx, &uly, NULL);
      if (pj_status)
        {
          QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
            (pj_strerrno (pj_status)).arg (mbr.min_x, 0, 'f', 11).arg (mbr.max_y, 0, 'f', 11).arg (ulx, 0, 'f', 11).arg (uly, 0, 'f', 11);
          QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
          exit (-1);
        }

      lrx = mbr.max_x * NV_DEG_TO_RAD;
      lry = mbr.min_y * NV_DEG_TO_RAD;
      pj_status = pj_transform (pfm_proj, bag_proj, 1, 1, &lrx, &lry, NULL);
      if (pj_status)
        {
          QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
            (pj_strerrno (pj_status)).arg (mbr.max_x, 0, 'f', 11).arg (mbr.min_y, 0, 'f', 11).arg (lrx, 0, 'f', 11).arg (lry, 0, 'f', 11);
          QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
          exit (-1);
        }

      urx = mbr.max_x * NV_DEG_TO_RAD;
      ury = mbr.max_y * NV_DEG_TO_RAD;
      pj_status = pj_transform (pfm_proj, bag_proj, 1, 1, &urx, &ury, NULL);
      if (pj_status)
        {
          QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
            (pj_strerrno (pj_status)).arg (mbr.max_x, 0, 'f', 11).arg (mbr.max_y, 0, 'f', 11).arg (urx, 0, 'f', 11).arg (ury, 0, 'f', 11);
          QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
          exit (-1);
        }

      if (llx < lrx)
        {
          proj_mbr.min_x = llx;
        }
      else
        {
          proj_mbr.min_x = lrx;
        }

      if (lrx > urx)
        {
          proj_mbr.max_x = lrx;
        }
      else
        {
          proj_mbr.max_x = urx;
        }

      if (lly < lry)
        {
          proj_mbr.min_y = lly;
        }
      else
        {
          proj_mbr.min_y = lry;
        }

      if (uly > ury)
        {
          proj_mbr.max_y = uly;
        }
      else
        {
          proj_mbr.max_y = ury;
        }


      bag_metadata.spatialRepresentationInfo->numberOfRows = bag_height = NINT ((proj_mbr.max_y - proj_mbr.min_y) / options.mbin_size + 0.05);
      bag_metadata.spatialRepresentationInfo->numberOfColumns = bag_width = NINT ((proj_mbr.max_x - proj_mbr.min_x) / options.mbin_size + 0.05);

      bag_metadata.spatialRepresentationInfo->rowResolution = options.mbin_size;
      bag_metadata.spatialRepresentationInfo->columnResolution = options.mbin_size;


      //  Make sure we have an exact number of bins.

      proj_mbr.max_x = proj_mbr.min_x + bag_width * options.mbin_size;
      proj_mbr.max_y = proj_mbr.min_y + bag_height * options.mbin_size;


      //  In order to make the output BAG have corner node (also known as grid) positioning we have to take
      //  half of a cell size off of the dimensions.  That's all we have to do.  We can still do all computations
      //  based on center node (also known as pixel) positioning.

      half_x = half_y = options.mbin_size / 2.0;


      //  Redefine the geodetic area from the northings and eastings so that projected and unprojected match.

      double x, y;

      x = proj_mbr.max_x - half_x;
      y = proj_mbr.max_y - half_y;
      pj_status = pj_transform (bag_proj, pfm_proj, 1, 1, &x, &y, NULL);
      if (pj_status)
        {
          QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
            (pj_strerrno (pj_status)).arg (proj_mbr.max_x - half_x, 0, 'f', 11).arg (proj_mbr.max_y - half_y, 0, 'f', 11).arg
            (x, 0, 'f', 11).arg (y, 0, 'f', 11);
          QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
          exit (-1);
        }
      mbr.max_x = x * NV_RAD_TO_DEG;
      mbr.max_y = y * NV_RAD_TO_DEG;

      x = proj_mbr.min_x + half_x;
      y = proj_mbr.min_y + half_y;
      pj_status = pj_transform (bag_proj, pfm_proj, 1, 1, &x, &y, NULL);
      if (pj_status)
        {
          QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
            (pj_strerrno (pj_status)).arg (proj_mbr.min_x + half_x, 0, 'f', 11).arg (proj_mbr.min_y + half_y, 0, 'f', 11).arg
            (x, 0, 'f', 11).arg (y, 0, 'f', 11);
          QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
          exit (-1);
        }
      mbr.min_x = x * NV_RAD_TO_DEG;
      mbr.min_y = y * NV_RAD_TO_DEG;

      bag_metadata.identificationInfo->westBoundingLongitude = mbr.min_x;
      bag_metadata.identificationInfo->eastBoundingLongitude = mbr.max_x;
      bag_metadata.identificationInfo->southBoundingLatitude = mbr.min_y;
      bag_metadata.identificationInfo->northBoundingLatitude = mbr.max_y;

      bag_metadata.spatialRepresentationInfo->llCornerX = proj_mbr.min_x + half_x;
      bag_metadata.spatialRepresentationInfo->urCornerX = proj_mbr.max_x - half_x;
      bag_metadata.spatialRepresentationInfo->llCornerY = proj_mbr.min_y + half_y;
      bag_metadata.spatialRepresentationInfo->urCornerY = proj_mbr.max_y - half_y;    
    }
  else
    {
      system.coordSys = Geodetic;


      bag_metadata.spatialRepresentationInfo->numberOfRows = bag_height = NINT ((mbr.max_y - mbr.min_y) / y_bin_size_degrees + 0.05);
      bag_metadata.spatialRepresentationInfo->numberOfColumns = bag_width = NINT ((mbr.max_x - mbr.min_x) / x_bin_size_degrees + 0.05);

      bag_metadata.spatialRepresentationInfo->rowResolution = y_bin_size_degrees;
      bag_metadata.spatialRepresentationInfo->columnResolution = x_bin_size_degrees;


      //  In order to make the output BAG have corner node (also known as grid) positioning we have to take
      //  half of a cell size off of the dimensions.  That's all we have to do.  We can still do all computations
      //  based on center node (also known as pixel) positioning.

      half_x = open_args.head.x_bin_size_degrees * 0.5;
      half_y = open_args.head.y_bin_size_degrees * 0.5;


      //  If the output is geodetic but has a different geodetic CRS we have to convert the bounds to the new CRS.

      if (io_crs_equal)
        {
          bag_metadata.identificationInfo->westBoundingLongitude = bag_metadata.spatialRepresentationInfo->llCornerX = mbr.min_x + half_x;
          bag_metadata.identificationInfo->eastBoundingLongitude = bag_metadata.spatialRepresentationInfo->urCornerX = mbr.max_x - half_x;
          bag_metadata.identificationInfo->southBoundingLatitude = bag_metadata.spatialRepresentationInfo->llCornerY = mbr.min_y + half_y;
          bag_metadata.identificationInfo->northBoundingLatitude = bag_metadata.spatialRepresentationInfo->urCornerY = mbr.max_y - half_y;
        }
      else
        {
          double x, y;

          x = mbr.min_x * NV_DEG_TO_RAD;
          y = mbr.min_y * NV_DEG_TO_RAD;
          pj_status = pj_transform (pfm_proj, bag_proj, 1, 1, &x, &y, NULL);
          if (pj_status)
            {
              QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
                (pj_strerrno (pj_status)).arg (mbr.min_x, 0, 'f', 11).arg (mbr.min_y, 0, 'f', 11).arg (x * NV_RAD_TO_DEG, 0, 'f', 11).arg (y * NV_RAD_TO_DEG, 0, 'f', 11);
              QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
              exit (-1);
            }
          x *= NV_RAD_TO_DEG;
          y *= NV_RAD_TO_DEG;


          bag_metadata.identificationInfo->westBoundingLongitude = bag_metadata.spatialRepresentationInfo->llCornerX = x + half_x;
          bag_metadata.identificationInfo->southBoundingLatitude = bag_metadata.spatialRepresentationInfo->llCornerY = y + half_y;

          x = mbr.max_x * NV_DEG_TO_RAD;
          y = mbr.max_y * NV_DEG_TO_RAD;
          pj_status = pj_transform (pfm_proj, bag_proj, 1, 1, &x, &y, NULL);
          if (pj_status)
            {
              QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
                (pj_strerrno (pj_status)).arg (mbr.max_x, 0, 'f', 11).arg (mbr.max_y, 0, 'f', 11).arg (x * NV_RAD_TO_DEG, 0, 'f', 11).arg (y * NV_RAD_TO_DEG, 0, 'f', 11);
              QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
              exit (-1);
            }
          x *= NV_RAD_TO_DEG;
          y *= NV_RAD_TO_DEG;

          bag_metadata.identificationInfo->eastBoundingLongitude = bag_metadata.spatialRepresentationInfo->urCornerX = x - half_x;
          bag_metadata.identificationInfo->northBoundingLatitude = bag_metadata.spatialRepresentationInfo->urCornerY = y - half_y;
        }
    }


  //  BAG metadata contact

  bag_metadata.contact->individualName = (u8 *) malloc (sizeof (u8) * options.poc_name.size ());
  strcpy ((char *) bag_metadata.contact->individualName, options.poc_name.toLatin1 ());

  bag_metadata.contact->organisationName = (u8 *) malloc (sizeof (u8) * options.source.size () + 1);
  strcpy ((char *) bag_metadata.contact->organisationName, options.source.toLatin1 ());

  bag_metadata.contact->positionName = (u8 *) malloc (sizeof (u8) * options.pi_title.size () + 1);
  strcpy ((char *) bag_metadata.contact->positionName, options.pi_title.toLatin1 ());

  bag_metadata.contact->role = (u8 *) malloc (sizeof (u8) * 20);
  strcpy ((char *) bag_metadata.contact->role, tr ("Point of Contact").toLatin1 ());


  if (!output_file_name.endsWith (".bag")) output_file_name.append (".bag");


  //  If the output bag already exists we have to remove it.

  if (QFile (output_file_name).exists ()) QFile (output_file_name).remove ();


  strcpy ((char *) name, output_file_name.toLatin1 ());


  bagError err;
  bagData data;
  bagHandle bagHandle;
  bagData opt_data_sep;
  bagTrackingItem trackItem;
  uint8_t *xmlBuffer;


  xmlBuffer = (uint8_t *) malloc (sizeof (uint8_t) * XML_METADATA_MAX_LENGTH);


  //  Set up the log array for scaling so we don't have to keep computing powers of ten in the main loop.  Note that I'm
  //  subtracting 1.0 from the results at 0 and going up to 0.0 at 100.  This is so that the curve is zero based.  It's not
  //  exactly a log curve but it's pretty darn close.

  for (int32_t i = 0 ; i < 100 ; i++) log_array[i] = pow (10.0L, ((double) i / 100.0)) - (1.0L * ((99.0L - (double) i) / 100.0L));


  memset (&data, 0, sizeof (data));


  //  If we are using the feature file to create an enhanced surface we have to create a weight array.

  if (options.enhanced && features)
    {
      //  Compute the pfmFeature search radius (if present) and save it in the radius array.

      radius = (double *) calloc (bfd_header.number_of_records, sizeof (double));
      if (radius == NULL)
        {
          QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Allocating radius memory : %1").arg (strerror (errno)));
          exit (-1);
        }

      for (uint32_t i = 0 ; i < bfd_header.number_of_records ; i++)
        {
          //  Make sure the feature that has been read is inside the bounds of the BAG being built.
          //  Also check the feature type and confidence.  If it is 0 it's invalid.  If it is 2 it was probably
          //  set with mosaicView and is non-sonar.  If it's 1 it's probably not very good.

          //  IMPORTANT NOTE: If you change this "if" statement, make sure you change it in all of the other
          //  places it is used.  They MUST match or you'll have crap data!  Just search for BFDATA_HYDROGRAPHIC.

          if (feature[i].feature_type == BFDATA_HYDROGRAPHIC && feature[i].confidence_level > 2 &&
              feature[i].longitude >= mbr.min_x && feature[i].longitude <= mbr.max_x &&
              feature[i].latitude >= mbr.min_y && feature[i].latitude <= mbr.max_y)
            {
              QString remarks = QString (feature[i].remarks);


              //  Compute the radius based on the diagonal of the bin size of features selected by pfmFeature.

              if (remarks.contains ("pfmFeature") && remarks.contains (", bin size "))
                {
                  //  This is the description of how we defined the search radius prior to adding the "max dist" output
                  //  to the feature remarks in pfmFeature.  If it is available we'll use the max dist otherwise we'll use
                  //  the method described below.

                  //  When running pfmFeature we use bin sizes of 3, 6, 12, and 24 meters (for IHO order 1).  To understand
                  //  how we apply the search radius for the bin sizes from pfmFeature you have to visualize possible locations
                  //  for the shoalest point in the center bin.  If the shoalest point is in the lower left corner of the 
                  //  bin then the maximum distance that a trigger point (nearest point that meets IHO criteria) can be from the 
                  //  shoal point (assuming 3 meter bins) is 7.071 meters.  That would be if the trigger point is in the upper
                  //  right corner of the upper right bin cell.  The effect of this would be that the maximum distance of the 
                  //  trigger point from the shoal point in the opposite direction would only be 2.83 meters.  To get a balanced
                  //  search radius to be used for our enhanced surface we will assume that the shoalest point is exactly in the
                  //  center of the center bin.  In that case the maximum distance in any direction to the trigger point would be
                  //  4.95 meters.  That is the sum of the diagonal of a square that is half the bin size plus the diagonal of
                  //  a square that is two thirds of the bin size (i.e. in the upper right corner of the upper right bin cell).


                  //  Check for the "max dist" string in the feature record.

                  if (remarks.contains (", max dist "))
                    {
                      radius[i] = remarks.section (',', 6, 6).section (' ', 3, 3).toDouble ();


                      //  Add the horizontal error to the radius.

                      radius[i] += (remarks.section (',', 4, 4).section ('/', 1, 1).section (' ', 1, 1).toDouble ());
                    }
                  else
                    {
                      double bin_size = remarks.section (',', 2, 2).section (' ', 3, 3).toDouble ();
                      double half = bin_size / 2.0L;
                      double two_thirds = bin_size * 2.0L / 3.0L;
                      double half_square = half * half;
                      double two_thirds_square = two_thirds * two_thirds;
                      radius[i] = sqrt (half_square + half_square) + sqrt (two_thirds_square + two_thirds_square);


                      //  Add the horizontal error to the radius.

                      radius[i] += (remarks.section (',', 4, 4).section ('/', 1, 1).section (' ', 1, 1).toDouble ());
                    }
                }
              else
                {
                  //  Set the radius for non-pfmFeature features.

                  radius[i] = options.non_radius;
                }
            }
        }


      //  Allocate the weight array.

      weight = (uint8_t **) calloc (bag_height, sizeof (uint8_t *));
      if (weight == NULL)
        {
            QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Allocating weight grid memory: %1").arg (strerror (errno)));
            exit (-1);
        }

      for (int32_t i = 0 ; i < bag_height ; i++)
        {
          weight[i] = (uint8_t *) calloc (bag_width, sizeof (uint8_t));
          if (weight[i] == NULL)
            {
              QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Allocating weight grid memory: %1").arg (strerror (errno)));
              exit (-1);
            }
        }


      //  Populate the weight array using the features.

      progress.wbar->setRange (0, bag_height);
      for (int32_t i = 0 ; i < bag_height ; i++)
        {
          progress.wbar->setValue (i);

          if (system.coordSys == UTM)
            {
              xy[0].y = proj_mbr.min_y + (double) i * options.mbin_size;
              xy[1].y = xy[0].y + options.mbin_size;

              northing = xy[0].y + half_y;
            }
          else
            {
              xy[0].y = mbr.min_y + (double) i * y_bin_size_degrees;
              xy[1].y = xy[0].y + y_bin_size_degrees;

              lat = xy[0].y + half_y;
            }

          for (int32_t j = 0 ; j < bag_width ; j++)
            {
              if (system.coordSys == UTM)
                {
                  xy[0].x = proj_mbr.min_x + (double) j * options.mbin_size;
                  xy[1].x = xy[0].x + options.mbin_size;

                  easting = xy[0].x + half_x;
                }
              else
                {
                  xy[0].x = mbr.min_x + (double) j * x_bin_size_degrees;
                  xy[1].x = xy[0].x + x_bin_size_degrees;

                  lon = xy[0].x + half_x;
                }

              double sum = 0.0;
              uint8_t hit = NVFalse;


              for (uint32_t k = 0 ; k < bfd_header.number_of_records ; k++)
                {
                  //  Make sure the feature that has been read is inside the bounds of the BAG being built.
                  //  Also check the feature type and confidence.  If it is 0 it's invalid.  If it is 2 it was probably
                  //  set with mosaicView and is non-sonar.  If it's 1 it's probably not very good.

                  //  IMPORTANT NOTE: If you change this "if" statement, make sure you change it in all of the other
                  //  places it is used.  They MUST match or you'll have crap data!  Just search for BFDATA_HYDROGRAPHIC.

                  if (feature[k].feature_type == BFDATA_HYDROGRAPHIC && feature[k].confidence_level > 2 &&
                      feature[k].longitude >= mbr.min_x && feature[k].longitude <= mbr.max_x &&
                      feature[k].latitude >= mbr.min_y && feature[k].latitude <= mbr.max_y)
                    {
                      //  Simple check first...  If it's in the same bin then we set the sum to 100.0 and move on.

                      if (feature[k].longitude >= xy[0].x && feature[k].longitude <= xy[1].x &&
                          feature[k].latitude >= xy[0].y && feature[k].latitude <= xy[1].y)
                        {
                          sum = 100.0;
                          hit = NVTrue;
                          break;
                        }


                      //  Now for the more complicated stuff...  We have to compute the distance from the feature to the
                      //  cell center to compute the weight.

                      double dist;

                      if (system.coordSys == UTM)
                        {
                          double x = feature[k].longitude * NV_DEG_TO_RAD;
                          double y = feature[k].latitude * NV_DEG_TO_RAD;
                          pj_status = pj_transform (pfm_proj, bag_proj, 1, 1, &x, &y, NULL);
                          if (pj_status)
                            {
                              QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg
                                (__LINE__).arg (__FUNCTION__).arg (pj_strerrno (pj_status)).arg (feature[k].longitude, 0, 'f', 11).arg
                                (feature[k].latitude, 0, 'f', 11).arg (x * NV_RAD_TO_DEG, 0, 'f', 11).arg (y * NV_RAD_TO_DEG, 0, 'f', 11);
                              QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
                              exit (-1);
                            }

                          dist = sqrt ((northing - y) * (northing - y) + (easting - x) * (easting - x));
                        }
                      else
                        {
                          pfm_geo_distance (pfm_handle, lat, lon, feature[k].latitude, feature[k].longitude, &dist);
                        }


                      //  If we're less than our prescribed distance away from any feature, we want to use a combination of
                      //  the minimum depth in the bin and the average depth for the bin.  We use a power of ten, or log,
                      //  curve to blend the two depths together.  Linear blending falls off too quickly and leaves you
                      //  with the same old spike sticking up (like we used to have with the tracking list).  The blending
                      //  works by taking 100 percent of the minimum depth in the bin in which the feature is located and
                      //  100 percent of the average depth in bins that are more than the feature search radius away from
                      //  the feature.  As we move away from the feature (but still inside the search radius) we include
                      //  more of the average and less of the minimum (based on the precomputed log curve).  If the search
                      //  radii of two features overlap we add the blended minimum depth components (not to exceed 100 percent).
                      //  If, at any point in the feature comparison for a single bin, we exceed 100 percent we stop doing
                      //  the feature comparison for that bin.  This saves us a bit of time.

                      if (dist < radius[k])
                        {
                          double percent = dist / radius[k];
                          int32_t index = NINT (percent * 100.0);

                          if (index < 100)
                            {
                              sum += 100.0 - (log_array[index] * 10.0);
                              hit = NVTrue;
                              if (sum >= 100.0) break;
                            }
                        }
                    }
                }

              if (hit) weight[i][j] = qMin (NINT (sum), 100);
            }

          qApp->processEvents ();
        }

      progress.wbar->setValue (bag_height);
      qApp->processEvents ();

      free (radius);
    }


  //  Have to have a processStep for each point in the tracking list if you want to create valid XML descriptions for a tracking list.

  int32_t count = 0;
  float value;


  //  Use features for tracking list.

  if (features)
    {
      //  First count the ones we want to include (valid, in the area, Hydrographic).

      for (uint32_t i = 0 ; i < bfd_header.number_of_records ; i++)
        {
          //  Make sure the feature that has been read is inside the bounds of the BAG being built.  Also check the feature type and
          //  the confidence.  If it is 0 it's invalid.  If it is 2 it was probably set with mosaicView and is non-sonar.  If it's 1
          //  it's probably not very good.

          //  IMPORTANT NOTE: If you change this "if" statement, make sure you change it in all of the other
          //  places it is used.  They MUST match or you'll have crap data!  Just search for BFDATA_HYDROGRAPHIC.

          if (feature[i].feature_type == BFDATA_HYDROGRAPHIC && feature[i].confidence_level > 2 &&
              feature[i].longitude >= mbr.min_x && feature[i].longitude <= mbr.max_x &&
              feature[i].latitude >= mbr.min_y && feature[i].latitude <= mbr.max_y)
            {
              count++;
            }
        }


      //  Now we have to allocate and populate the data quality section of the metadata

      bag_metadata.dataQualityInfo->numberOfProcessSteps = count;

      bag_metadata.dataQualityInfo->lineageProcessSteps = (BAG_PROCESS_STEP *) malloc (bag_metadata.dataQualityInfo->numberOfProcessSteps *
                                                                                       sizeof(BAG_PROCESS_STEP));

      for (uint32_t i = 0 ; i < bag_metadata.dataQualityInfo->numberOfProcessSteps ; i++)
        {
          bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources = (BAG_SOURCE *) malloc (sizeof(BAG_SOURCE));
          bag_metadata.dataQualityInfo->lineageProcessSteps[i].numberOfSources = 1;
          bag_metadata.dataQualityInfo->lineageProcessSteps[i].numberOfProcessors =1;

          bag_metadata.dataQualityInfo->lineageProcessSteps[i].processors =
            (BAG_RESPONSIBLE_PARTY *) malloc (bag_metadata.dataQualityInfo->lineageProcessSteps[i].numberOfProcessors * sizeof (BAG_RESPONSIBLE_PARTY));

          for (uint32_t j = 0; j < bag_metadata.dataQualityInfo->lineageProcessSteps[i].numberOfSources ; j++)
            {
              bag_metadata.dataQualityInfo->lineageProcessSteps[i].processors[j].individualName = (u8 *) malloc (sizeof (u8) * 128);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].processors[j].individualName, "\0");
              bag_metadata.dataQualityInfo->lineageProcessSteps[i].processors[j].positionName = (u8 *) malloc (sizeof (u8) * 128);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].processors[j].positionName, "\0");
              bag_metadata.dataQualityInfo->lineageProcessSteps[i].processors[j].organisationName = (u8 *) malloc (sizeof (u8) * 128);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].processors[j].organisationName, "\0");
              bag_metadata.dataQualityInfo->lineageProcessSteps[i].processors[j].role = (u8 *) malloc (sizeof (u8) * 52);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].processors[j].role, "\0");      
            }

          bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources->responsibleParties =
            (BAG_RESPONSIBLE_PARTY *) malloc (bag_metadata.dataQualityInfo->lineageProcessSteps[i].numberOfSources * sizeof (BAG_RESPONSIBLE_PARTY));

          for (uint32_t j = 0; j < bag_metadata.dataQualityInfo->lineageProcessSteps[i].numberOfSources ; j++)
            {
              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties->individualName = (u8 *) malloc (sizeof (u8) * 128);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties->individualName, "\0");
              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties->positionName = (u8 *) malloc (sizeof (u8) * 128);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties->positionName, "\0");
              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties->organisationName =
                (u8 *) malloc (sizeof (u8) * 128);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties->organisationName, "\0");
              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties->role = (u8 *) malloc (sizeof (u8) * 128);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties->role, "\0");

              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].date = (u8 *) malloc (sizeof (u8) * 30);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].date, "\0");

              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].dateType = (u8 *) malloc (sizeof (u8) * 30);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].dateType, "publication");

              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].description = (u8 *) malloc (sizeof (u8) * 30);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].description, "NAVO PFM");

              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].numberOfResponsibleParties =1;

              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties[0].individualName =
                (u8 *) malloc (sizeof (u8) * 52);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties[0].individualName,
                      "Commander of the NAVY");

              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties[0].organisationName =
                (u8 *) malloc (sizeof (u8) * 30);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties[0].organisationName, "NAVOCEANO");

              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties[0].positionName = (u8 *) malloc (sizeof (u8) * 30);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties[0].positionName, " ");

              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties[0].role = (u8 *) malloc (sizeof (u8) * 30);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].responsibleParties[0].role, " ");

              bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].title = (u8 *) malloc (sizeof (u8) * 64);
              strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].lineageSources[j].title, "pfmBag");
            }


          //  Set the date and time.

          int32_t year, jday, month, mday, hour, minute;
          float second;
          cvtime (feature[i].event_tv_sec, feature[i].event_tv_nsec, &year, &jday, &hour, &minute, &second);
          jday2mday (year, jday, &month, &mday);
          month++;

          char tmp_string[128];
          sprintf (tmp_string,  "%04d-%02d-%02dT%02d:%02d:%02dZ", year + 1900, month, mday, hour, minute, NINT (second));

          bag_metadata.dataQualityInfo->lineageProcessSteps[i].dateTime = (u8 *) malloc (sizeof (u8) * strlen (tmp_string) + 1);
          strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].dateTime, tmp_string);


          QString remarks = QString (feature[i].remarks);


          //  Put the description and remarks into the XML data.

          QString string0 (feature[i].description);

          QString string1 (feature[i].remarks);

          QString string2 ("");


          //  If the BFDATA_RECORD "parent_record" field is set to anything other than zero, then it is a child record of the
          //  (parent_record - 1) feature.

          if (feature[i].parent_record) string2 = QString ("Child of tracking list entry #%1").arg (feature[i].parent_record - 1);


          QString new_string;

          if (string0.isEmpty () && string1.isEmpty () && string2.isEmpty ())
            {
              new_string = "No description available";
            }
          else
            {
              new_string = string0 + " :: " + string1 + "::" + string2;
            }


          bag_metadata.dataQualityInfo->lineageProcessSteps[i].description = (u8 *) malloc (sizeof (u8) * new_string.size () + 1);
          strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].description, new_string.toLatin1 ());


          //  Finally, set the trackingId.

          sprintf (tmp_string, "%d", i);
          bag_metadata.dataQualityInfo->lineageProcessSteps[i].trackingId = (u8 *) malloc (sizeof (u8) * 10);
          strcpy ((char *) bag_metadata.dataQualityInfo->lineageProcessSteps[i].trackingId, tmp_string);
        }
    }




  bag_metadata.dataQualityInfo->scope = (u8 *) malloc (sizeof (u8) * 30);
  strcpy ((char *) bag_metadata.dataQualityInfo->scope, "dataset");
  err = bagInitDefinition (&data.def, &bag_metadata);


  //  Create the XML metadata

  int32_t len = bagExportMetadataToXmlBuffer (&bag_metadata, &xmlBuffer);


  //  A new BAG file is being created, so set the correct version on the bagData so we can correctly decode the metadata.

  strcpy ((char *) data.version, BAG_VERSION);


  /*  write buffer to file - test

  char tempname[4028];
  FILE *infile;
  strcpy (tempname, "temp_file.xml");
  if ((infile = fopen (tempname, "w")) == NULL)
    {
      string.sprintf (tr ("%s %s %s %d - Error opening file %s").toLatin1 (), options.progname, __FILE__, __FUNCTION__, __LINE__, tempname);
      QMessageBox::critical (this, "pfmBag", string);
      exit (-1);
    }

  fprintf (infile, "%s", xmlBuffer);
  fclose (infile);
  */


  //  Allocate the metadata space.

  data.metadata = (u8 *) malloc ((sizeof (u8)) * (len + 1));
  strcpy ((char *) data.metadata, (char *) xmlBuffer); 


  //  Set data compression.

  data.compressionLevel = 1;


  //  Create the BAG file.

  if ((err = bagFileCreate (name, &data, &bagHandle)) != BAG_SUCCESS)
    {
      string = tr ("Error creating BAG file");

      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

      QMessageBox::warning (this, tr ("pfmBag Error"), string);


      if (options.enhanced)
        {
          for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
          free (weight);
        }


      free (xmlBuffer);

      exit (-1);
    }


  //  Allocate the elevation array.

  float *elevation = (float *) calloc (bag_width, sizeof (float));

  if (elevation == NULL)
    {
      string.sprintf (tr ("%s %s %s %d - elevation - %s").toLatin1 (), options.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
      QMessageBox::critical (this, "pfmBag", string);
      exit (-1);
    }


  //  Allocate the uncertainty array.

  float *uncert = NULL;
  uncert = (float *) calloc (bag_width, sizeof (float));

  if (uncert == NULL)
    {
      string.sprintf (tr ("%s %s %s %d - uncertainty - %s").toLatin1 (), options.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
      QMessageBox::critical (this, "pfmBag", string);
      exit (-1);
    }


  //  Allocate the optional elevation solution group array;

  bagOptElevationSolutionGroup *optsol = NULL;

  optsol = (bagOptElevationSolutionGroup *) calloc (bag_width, sizeof (bagOptElevationSolutionGroup));

  if (optsol == NULL)
    {
      string.sprintf (tr ("%s %s %s %d - optional elevation solution group - %s").toLatin1 (), options.progname, __FILE__, __FUNCTION__, __LINE__,
                      strerror (errno));
      QMessageBox::critical (this, "pfmBag", string);
      exit (-1);
    }


  bagGetDataPointer (bagHandle)->opt[Elevation_Solution_Group].nrows = bag_height;
  bagGetDataPointer (bagHandle)->opt[Elevation_Solution_Group].ncols = bag_width;


  //  bagCreateElevationSolutionGroup will create the hid_t needed by HDF5 and will store it in
  //  bagGetDataPointer (bagHandle)->opt[Elevation_Solution_Group].datatype so we don't have to specify it above.

  if ((err = bagCreateElevationSolutionGroup (bagHandle, bagGetDataPointer (bagHandle))) != BAG_SUCCESS)
    {
      string = tr ("Error creating Elevation Solution Group optional dataset");

      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

      QMessageBox::warning (this, tr ("pfmBag Error"), string);


      free (elevation);
      free (uncert);
      free (optsol);

      if (options.enhanced)
        {
          for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
          free (weight);
        }

      free (xmlBuffer);

      exit (-1);
    }


  if ((err = bagAllocArray (bagHandle, 0, 0, bag_height - 1, bag_width - 1, Elevation_Solution_Group)) != BAG_SUCCESS)
    {
      string = tr ("Error allocating Elevation Solution Group optional dataset");

      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

      QMessageBox::warning (this, tr ("pfmBag Error"), string);


      free (elevation);
      free (uncert);
      free (optsol);

      if (options.enhanced)
        {
          for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
          free (weight);
        }

      free (xmlBuffer);

      exit (-1);
    }


  //  If we're using the CUBE surface, allocate the cube node array.

  bagOptNodeGroup *cube = NULL;
  if (options.surface == CUBE_SURFACE)
    {
      //  Allocate the cube node array.

      cube = (bagOptNodeGroup *) calloc (bag_width, sizeof (bagOptNodeGroup));

      if (cube == NULL)
        {
          string.sprintf (tr ("%s %s %s %d - cube node - %s").toLatin1 (), options.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
          QMessageBox::critical (this, "pfmBag", string);
          exit (-1);
        }


      bagGetDataPointer (bagHandle)->opt[Node_Group].nrows = bag_height;
      bagGetDataPointer (bagHandle)->opt[Node_Group].ncols = bag_width;


      //  bagCreateNodeGroup will create the hid_t needed by HDF5 and will store it in bagGetDataPointer (bagHandle)->opt[Node_Group].datatype
      //  so we don't have to specify it above.

      if ((err = bagCreateNodeGroup (bagHandle, bagGetDataPointer (bagHandle))) != BAG_SUCCESS)
        {
          string = tr ("Error creating Node Group optional dataset");

          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

          QMessageBox::warning (this, tr ("pfmBag Error"), string);


          free (elevation);
          free (uncert);
          free (optsol);

          free (cube);

          if (options.enhanced)
            {
              for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
              free (weight);
            }

          free (xmlBuffer);

          exit (-1);
        }


      if ((err = bagAllocArray (bagHandle, 0, 0, bag_height - 1, bag_width - 1, Node_Group)) != BAG_SUCCESS)
        {
          string = tr ("Error allocating Node Group optional dataset");

          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

          QMessageBox::warning (this, tr ("pfmBag Error"), string);


          free (elevation);
          free (uncert);
          free (optsol);

          free (cube);

          if (options.enhanced)
            {
              for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
              free (weight);
            }

          free (xmlBuffer);

          exit (-1);
        }
    }


  //  Store the values in the BAG.

  progress.mbar->setRange (0, bag_height);

  double py[2] = {0.0, 0.0};


  //  Figure out where (if anywhere) the final uncertainty, hypothesis strength, and number of hypotheses attributes are stored.

  int32_t fu_attr = -1;
  int32_t hs_attr = -1;
  int32_t nh_attr = -1;
  for (int32_t i = 0 ; i < open_args.head.num_bin_attr ; i++)
    {
      if (strstr (open_args.head.bin_attr_name[i], "###5")) fu_attr = i;
      if (strstr (open_args.head.bin_attr_name[i], "###2")) hs_attr = i;
      if (strstr (open_args.head.bin_attr_name[i], "###0")) nh_attr = i;
    }


  //  Loop for the height of the PFM.

  for (int32_t i = 0 ; i < bag_height ; i++)
    {
      progress.mbar->setValue (i);

      if (system.coordSys == UTM)
        {
          py[0] = proj_mbr.min_y + (double) i * options.mbin_size;
          py[1] = py[0] + options.mbin_size;
        }
      else
        {
          xy[0].y = mbr.min_y + (double) i * y_bin_size_degrees;
          xy[1].y = xy[0].y + y_bin_size_degrees;
        }


      //  Loop for the width of the PFM.

      for (int32_t j = 0 ; j < bag_width ; j++)
        {
          NV_I32_COORD2 coord[2];


          //  Determine the range of the cell coordinates of the cells that have data in the output bin.

          if (system.coordSys == UTM)
            {
              xy[0].x = proj_mbr.min_x + (double) j * options.mbin_size;
              xy[1].x = xy[0].x + options.mbin_size;

              double x = xy[0].x;
              double y = py[0];
              pj_status = pj_transform (bag_proj, pfm_proj, 1, 1, &x, &y, NULL);
              if (pj_status)
                {
                  QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
                    (pj_strerrno (pj_status)).arg (xy[0].x, 0, 'f', 11).arg (py[0], 0, 'f', 11).arg (x * NV_RAD_TO_DEG, 0, 'f', 11).arg (y * NV_RAD_TO_DEG, 0, 'f', 11);
                  QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
                  exit (-1);
                }
              xy[0].x = x * NV_RAD_TO_DEG;
              xy[0].y = y * NV_RAD_TO_DEG;

              x = xy[1].x;
              y = py[1];
              pj_status = pj_transform (bag_proj, pfm_proj, 1, 1, &x, &y, NULL);
              if (pj_status)
                {
                  QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg (__LINE__).arg (__FUNCTION__).arg
                    (pj_strerrno (pj_status)).arg (xy[1].x, 0, 'f', 11).arg (py[1], 0, 'f', 11).arg (x * NV_RAD_TO_DEG, 0, 'f', 11).arg (y * NV_RAD_TO_DEG, 0, 'f', 11);
                  QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
                  exit (-1);
                }
              xy[1].x = x * NV_RAD_TO_DEG;
              xy[1].y = y * NV_RAD_TO_DEG;
            }
          else
            {
              xy[0].x = mbr.min_x + (double) j * x_bin_size_degrees;
              xy[1].x = xy[0].x + x_bin_size_degrees;
            }

          compute_index_ptr (xy[0], &coord[0], &open_args.head);
          compute_index_ptr (xy[1], &coord[1], &open_args.head);


          elevation[j] = NULL_ELEVATION;
          uncert[j] = NULL_UNCERTAINTY;

          optsol[j].stddev = NULL_STD_DEV;
          optsol[j].shoal_elevation = NULL_GENERIC;
          optsol[j].num_soundings = NULL_GENERIC;

          if (options.surface == CUBE_SURFACE)
            {
              cube[j].hyp_strength = NULL_GENERIC;
              cube[j].num_hypotheses = NULL_GENERIC;
            }


          double sum = 0.0;
          double sum2 = 0.0;
          double uncert_sum = 0.0;
          double uncert_sum2 = 0.0;
          double min_uncert = 9999999999.0;
          int32_t count = 0;
          double max_z = -999999999.0;
          double min_z = 999999999.0;


          //  If we're running a CUBE surface we can't change the bin size or select the uncertainty type.  These will be hard-wired.

          if (options.surface == CUBE_SURFACE)
            {
              BIN_RECORD bin;


              //  Check for out of bounds (can happen when going to UTM).

              if (coord[0].x >= 0 && coord[0].y >= 0 && coord[0].x < open_args.head.bin_width && coord[0].y < open_args.head.bin_height)
                {
                  read_bin_record_index (pfm_handle, coord[0], &bin);

                  if (bin.validity & PFM_DATA)
                    {
                      sum = bin.avg_filtered_depth;
                      min_z = bin.min_filtered_depth;
                      min_uncert = uncert_sum = bin.attr[fu_attr];
                      cube[j].hyp_strength = bin.attr[hs_attr];
                      cube[j].num_hypotheses = bin.attr[nh_attr];


                      DEPTH_RECORD *depth;
                      int32_t numrecs;

                      if (!read_depth_array_index (pfm_handle, coord[0], &depth, &numrecs))
                        {
                          for (int32_t p = 0 ; p < numrecs ; p++)
                            {
                              if (!(depth[p].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
                                {
                                  //  If we are creating the enhanced surface we need to get the uncertainty of the minimum depth.

                                  if (options.enhanced && depth[p].xyz.z <= min_z) min_uncert = depth[p].vertical_error;


                                  count++;
                                }
                            }

                          free (depth);
                        }
                    }
                }
            }
          else
            {
              //  Loop over the height and width of the covering cells.

              for (int32_t m = coord[0].y ; m <= coord[1].y ; m++)
                {
                  if (m >= 0 && m < open_args.head.bin_height)
                    {
                      NV_I32_COORD2 icoord;
                      icoord.y = m;

                      for (int32_t n = coord[0].x ; n <= coord[1].x ; n++)
                        {
                          if (n >= 0 && n < open_args.head.bin_width)
                            {
                              icoord.x = n;


                              DEPTH_RECORD *depth;
                              int32_t numrecs;

                              if (!read_depth_array_index (pfm_handle, icoord, &depth, &numrecs))
                                {
                                  for (int32_t p = 0 ; p < numrecs ; p++)
                                    {
                                      if ((!(depth[p].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) &&
                                          depth[p].xyz.x >= xy[0].x && depth[p].xyz.x <= xy[1].x &&
                                          depth[p].xyz.y >= xy[0].y && depth[p].xyz.y <= xy[1].y)
                                        {

                                          //  Get the minimum depth and the uncertainty of that depth.

                                          if (depth[p].xyz.z <= min_z)
                                            {
                                              min_uncert = depth[p].vertical_error;
                                              min_z = depth[p].xyz.z;
                                            }

                                          max_z = qMax (max_z, depth[p].xyz.z);

                                          sum += depth[p].xyz.z;
                                          sum2 += depth[p].xyz.z * depth[p].xyz.z;
                                          uncert_sum += depth[p].vertical_error;
                                          uncert_sum2 += depth[p].vertical_error * depth[p].vertical_error;
                                          count++;
                                        }
                                    }

                                  free (depth);
                                }
                            }
                        }
                    }
                }
            }


          qApp->processEvents ();


          if (count)
            {
              double avg = sum / (double) count;

              switch (options.uncertainty)
                {
                case STD_UNCERT:
                  uncert[j] = 0.0;

                  if (count > 1)
                    {
                      double variance = ((sum2 - ((double) count * (pow (avg, 2.0)))) / ((double) count - 1.0));
                      if (variance >= 0.0) uncert[j] = sqrt (variance);
                    }
                  break;

                case TPE_UNCERT:
                  if (options.enhanced)
                    {
                      float weight1 = (100.0 - (float) weight[i][j]) / 100.0;
                      float weight2 = (float) weight[i][j] / 100.0;
                      uncert[j] = -((sqrt (uncert_sum2 / (double) count)) * weight1 + min_uncert * weight2);
                    }
                  else
                    {
                      uncert[j] = sqrt (uncert_sum2 / (double) count);
                    }
                  break;

                case FIN_UNCERT:
                  if (options.enhanced)
                    {
                      float weight1 = (100.0 - (float) weight[i][j]) / 100.0;
                      float weight2 = (float) weight[i][j] / 100.0;
                      uncert[j] = -(uncert_sum * weight1 + min_uncert * weight2);
                    }
                  else
                    {
                      uncert[j] = uncert_sum;
                    }
                  break;
                }


              switch (options.surface)
                {
                case MIN_SURFACE:
                  elevation[j] = -min_z + options.elev_off;
                  break;

                case MAX_SURFACE:
                  elevation[j] = -max_z + options.elev_off;
                  break;

                case AVG_SURFACE:
                  if (options.enhanced)
                    {
                      float weight1 = (100.0 - (float) weight[i][j]) / 100.0;
                      float weight2 = (float) weight[i][j] / 100.0;
                      elevation[j] = -(avg * weight1 + min_z * weight2) + options.elev_off;
                    }
                  else
                    {
                      elevation[j] = -avg + options.elev_off;
                    }
                  break;

                case CUBE_SURFACE:
                  if (options.enhanced)
                    {
                      float weight1 = (100.0 - (float) weight[i][j]) / 100.0;
                      float weight2 = (float) weight[i][j] / 100.0;
                      elevation[j] = -(sum * weight1 + min_z * weight2) + options.elev_off;
                    }
                  else
                    {
                      elevation[j] = -sum + options.elev_off;
                    }
                  break;
                }


              optsol[j].shoal_elevation = -min_z;
              optsol[j].num_soundings = count;

              if (count > 1)
                {
                  double variance = ((sum2 - ((double) count * (pow (avg, 2.0)))) / ((double) count - 1.0));
                  if (variance >= 0.0) optsol[j].stddev = sqrt (variance);
                }
            }
        }


      if ((err = bagWriteRow (bagHandle, i, 0, bag_width - 1, Elevation, (void *) elevation)) != BAG_SUCCESS)
        {
          string = tr ("Error writing elevation at row %1").arg (i);

          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

          QMessageBox::warning (this, tr ("pfmBag Error"), string);


          free (elevation);
          free (uncert);
          free (optsol);

          if (options.surface == CUBE_SURFACE) free (cube);

          if (options.enhanced)
            {
              for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
              free (weight);
            }

          free (xmlBuffer);

          exit (-1);
        }


      if ((err = bagWriteRow (bagHandle, i, 0, bag_width - 1, Uncertainty, (void *) uncert)) != BAG_SUCCESS)
        {
          string = tr ("Error writing uncertainty at row %1").arg (i);

          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

          QMessageBox::warning (this, tr ("pfmBag Error"), string);


          free (elevation);
          free (uncert);
          free (optsol);

          if (options.surface == CUBE_SURFACE) free (cube);

          if (options.enhanced)
            {
              for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
              free (weight);
            }

          free (xmlBuffer);

          exit (-1);
        }


      if ((err = bagWriteRow (bagHandle, i, 0, bag_width - 1, Elevation_Solution_Group, (void *) optsol)) != BAG_SUCCESS)
        {
          string = tr ("Error writing optional solution group node at row %1").arg (i);

          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

          QMessageBox::warning (this, tr ("pfmBag Error"), string);


          free (elevation);
          free (uncert);
          free (optsol);

          if (options.surface == CUBE_SURFACE) free (cube);

          if (options.enhanced)
            {
              for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
              free (weight);
            }

          free (xmlBuffer);

          exit (-1);
        }


      if (options.surface == CUBE_SURFACE)
        {
          if ((err = bagWriteRow (bagHandle, i, 0, bag_width - 1, Node_Group, (void *) cube)) != BAG_SUCCESS)
            {
              string = tr ("Error writing CUBE node at row %1").arg (i);

              u8 *errstr;

              if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

              QMessageBox::warning (this, tr ("pfmBag Error"), string);


              free (elevation);
              free (uncert);
              free (optsol);

              if (options.surface == CUBE_SURFACE) free (cube);

              if (options.enhanced)
                {
                  for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
                  free (weight);
                }

              free (xmlBuffer);

              exit (-1);
            }
        }
    }


  if (options.enhanced)
    {
      for (int32_t i = 0 ; i < bag_height ; i++) free (weight[i]);
      free (weight);
    }


  //  Free the arrays.

  free (elevation);
  free (uncert);
  free (optsol);

  if (options.surface == CUBE_SURFACE) free (cube);


  progress.mbar->setValue (bag_height);
  qApp->processEvents ();


  //  Put the features in the tracking list.

  count = 0;


  if (features)
    {
      progress.gbar->setRange (0, bfd_header.number_of_records);

      for (uint32_t i = 0 ; i < bfd_header.number_of_records ; i++)
        {
          progress.gbar->setValue (i);
          qApp->processEvents ();


          //  Make sure the feature that has been read is inside the bounds of the BAG being built.  Also check the feature type and
          //  the confidence.  If it is 0 it's invalid.  If it is 2 it was probably set with mosaicView and is non-sonar.  If it's 1
          //  it's probably not very good.

          //  IMPORTANT NOTE: If you change this "if" statement, make sure you change it in all of the other
          //  places it is used.  They MUST match or you'll have crap data!  Just search for BFDATA_HYDROGRAPHIC.

          if (feature[i].feature_type == BFDATA_HYDROGRAPHIC && feature[i].confidence_level > 2 &&
              feature[i].longitude >= mbr.min_x && feature[i].longitude <= mbr.max_x &&
              feature[i].latitude >= mbr.min_y && feature[i].latitude <= mbr.max_y)
            {
              QString remarks = QString (feature[i].remarks);

              if (system.coordSys == UTM)
                {
                  double x = feature[i].longitude * NV_DEG_TO_RAD;
                  double y = feature[i].latitude * NV_DEG_TO_RAD;
                  pj_status = pj_transform (pfm_proj, bag_proj, 1, 1, &x, &y, NULL);
                  if (pj_status)
                    {
                      QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg
                        (__LINE__).arg (__FUNCTION__).arg (pj_strerrno (pj_status)).arg (feature[i].longitude, 0, 'f', 11).arg (feature[i].latitude, 0, 'f', 11).arg
                        (x, 0, 'f', 11).arg (y, 0, 'f', 11);
                      QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
                      exit (-1);
                    }

                  trackItem.row = NINT (((y - proj_mbr.min_y) / options.mbin_size) + 0.5) ;
                  trackItem.col = NINT (((x - proj_mbr.min_x) / options.mbin_size) + 0.5) ;
                }
              else
                {
                  trackItem.row = NINT (((feature[i].latitude - mbr.min_y) / y_bin_size_degrees) + 0.5) ;
                  trackItem.col = NINT (((feature[i].longitude - mbr.min_x) / x_bin_size_degrees) + 0.5) ;
                }

              if ((err = bagReadNode (bagHandle, trackItem.row, trackItem.col, Elevation, (void *) &value)) != BAG_SUCCESS)
                {
                  string = tr ("Error reading elevation node at %1, %2").arg (trackItem.row).arg (trackItem.col);

                  u8 *errstr;

                  if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

                  QMessageBox::warning (this, tr ("pfmBag Error"), string);


                  free (xmlBuffer);
                  exit (-1);
                }


              trackItem.depth = -value;


              //  OK.  Let's talk about BAG.  The track_code is just a number that's supposed to tell you what the tracking list item is
              //  all about.  Apparently BAG wants to use bagDesignatedSndg to denote a selected IHO feature (in NAVO's version of GSF
              //  processing, this would be NV_GSF_SELECTED_DESIGNATED).  In PFM we use PFM_SELECTED_FEATURE for these.  We use
              //  PFM_DESIGNATED_SOUNDING to indicate a selected sounding that needs to be saved into the tracking list but *isn't* an
              //  IHO selected feature.  In BFD we have parent and child features.  Parent features are always IHO features
              //  (PFM_SELECTED_FEATURE).  Child features will be PFM_DESIGNATED_SOUNDINGS.  As far as I can tell there is no track_code
              //  value for this kind of point in either BAG 1.5.3 or the (yet to be implemented here) BAG 1.6.0.  The only available
              //  values are, in enum order: bagManualEdit, bagDesignatedSndg, bagRecubedSurfaces, and bagDeleteNode.  Obviosly, none of
              //  these will work for our children (or our children's, children's, children [Moody Blues reference] for that matter).
              //  So, I'm going to set the track_code to 129 to indicate a child (in BFD), a PFM_DESIGNATED_SOUNDING (in PFM), a
              //  NV_GSF_SELECTED_SPARE_1 (in GSF), and a CZMIL_RETURN_DESIGNATED_SOUNDING (in CZMIL CPF).  If the BFDATA_RECORD
              //  "parent_record" field is set to anything other than zero, then it is a child record of the (parent_record - 1) feature.

              if (feature[i].parent_record)
                {
                  trackItem.track_code = 129;
                }
              else
                {
                  trackItem.track_code = bagDesignatedSndg;
                }


              //  Now, list_series.  According to the BAG documentation (HA!  I had to look at the code), the list_series is the
              //  "index number indicating the item in the metadata that describes the modifications".  What the hell does that
              //  mean?  What item?  What modifications?  Oh, I get it now.  It was intuitively obvious to the most casual
              //  observer.  What they mean is that this points to the bag_metadata.dataQualityInfo->lineageProcessSteps entry
              //  that has information about this tracking list item, why it's here, and what modifications were made.  In other
              //  words, bag_metadata.dataQualityInfo->lineageProcessSteps[trackItem.list_series].  Boy do I feel dumb now!
              //  It was so simple, like the jitterbug it plumb evaded me [Jimmy Buffett reference].

              trackItem.list_series = i;


              //  Write the tracking list item.

              if ((err = bagWriteTrackingListItem (bagHandle, &trackItem)) != BAG_SUCCESS)
                {
                  string = tr ("Error adding tracking list item");

                  u8 *errstr;

                  if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

                  QMessageBox::warning (this, tr ("pfmBag Error"), string);


                  free (xmlBuffer);
                  exit (-1);
                }


              //  Write the modified nodes.

              value = -feature[i].depth;
              if ((err = bagWriteNode (bagHandle, trackItem.row, trackItem.col, Elevation, (void *) &value)) != BAG_SUCCESS)
                {
                  string = tr ("Error writing elevation node at %1,%2").arg (trackItem.row).arg (trackItem.col);

                  u8 *errstr;

                  if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

                  QMessageBox::warning (this, tr ("pfmBag Error"), string);


                  free (xmlBuffer);
                  exit (-1);
                }
            }
        }

      progress.gbar->setValue (bfd_header.number_of_records);


      //  Close the bfd file here.  This frees the short feature structure.

      binaryFeatureData_close_file (bfd_handle);
    }


  if ((err = bagUpdateSurface (bagHandle, Elevation)) != BAG_SUCCESS)
    {
      string = tr ("Error updating elevation surface");

      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

      QMessageBox::warning (this, tr ("pfmBag Error"), string);


      free (xmlBuffer);
      exit (-1);
    }


  if ((err = bagUpdateSurface (bagHandle, Uncertainty)) != BAG_SUCCESS)
    {
      string = tr ("Error updating uncertainty surface");

      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

      QMessageBox::warning (this, tr ("pfmBag Error"), string);


      free (xmlBuffer);
      exit (-1);
    }


  if ((err = bagUpdateSurface (bagHandle, Elevation_Solution_Group)) != BAG_SUCCESS)
    {
      string = tr ("Error updating optional elevation solution group surface");

      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

      QMessageBox::warning (this, tr ("pfmBag Error"), string);


      free (xmlBuffer);
      exit (-1);
    }

  bagFreeArray (bagHandle, Elevation_Solution_Group);


  if (options.surface == CUBE_SURFACE)
    {
      if ((err = bagUpdateSurface (bagHandle, Node_Group)) != BAG_SUCCESS)
        {
          string = tr ("Error updating CUBE node surface");

          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

          QMessageBox::warning (this, tr ("pfmBag Error"), string);


          free (xmlBuffer);
          exit (-1);
        }

      bagFreeArray (bagHandle, Node_Group);
    }


  //  If we added any features to the tracking list we need to redo the XML metadata.

  if (bag_metadata.dataQualityInfo->numberOfProcessSteps)
    {
      bagGetDataPointer (bagHandle)->metadata = xmlBuffer;


      if ((err = bagWriteXMLStream (bagHandle)) != BAG_SUCCESS)
        {
          string = tr ("Error writing XML stream");

          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

          QMessageBox::warning (this, tr ("pfmBag Error"), string);


          free (xmlBuffer);
          exit (-1);
        }
    }



  //  Adding optional separation surface if requested.

  if (!sep_file_name.isEmpty ())
    {
      bagVerticalCorrectorDef bvc;

      if (sep_file_name.endsWith (".ch2"))
        {
          if ((sep_handle = chrtr2_open_file (sep_file, &sep_header, CHRTR2_READONLY)) < 0)
            {
              QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Unable to open CHRTR2 separation file %1\nReason: %2").arg (sep_file_name).arg
                                     (QString (chrtr2_strerror ())));
              exit (-1);
            }

        }    

      memset (&opt_data_sep, 0, sizeof(opt_data_sep));

      strcpy (sep_file, sep_file_name.toLatin1 ());


      //  Copy most of the default setup for the main BAG.

      opt_data_sep.def = data.def;


      //  Check for .ch2 extension.

      if (sep_file_name.endsWith (".ch2"))
        {
          if ((sep_handle = chrtr2_open_file (sep_file, &sep_header, CHRTR2_READONLY)) < 0)
            {
              QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Unable to open CHRTR2 separation file %1\nReason: %2").arg (sep_file_name).arg
                                     (QString (chrtr2_strerror ())));
              exit (-1);
            }

          opt_data_sep.opt[Surface_Correction].ncols = sep_header.width;
          opt_data_sep.opt[Surface_Correction].nrows = sep_header.height;
          bvc.nodeSpacingX = sep_header.lon_grid_size_degrees;
          bvc.nodeSpacingY = sep_header.lat_grid_size_degrees;
          bvc.swCornerX = sep_header.mbr.wlon;
          bvc.swCornerY = sep_header.mbr.slat;
        }
      else
        {
          if ((sep_fp = fopen (sep_file, "r")) == NULL)
            {
              QMessageBox::critical (this, tr ("pfmBag Error"), tr ("Unable to open ASCII separation file %1\nReason: %2").arg (sep_file_name).arg
                                     (QString (strerror (errno))));
              exit (-1);
            }


          ngets (sep_string, sizeof (sep_string), sep_fp);

          if (!strstr (sep_string, "LAT,LONG,Z0,Z1"))
            {
              QMessageBox::critical (this, tr ("pfmBag Error"), tr ("ASCII separation file %1 format incorrect").arg (sep_file_name));
              exit (-1);
            }


          //  Unfortunately, for ASCII files we have to read the whole file to determine the width, height, and other stuff.

          int32_t pos = ftell (sep_fp);
          int32_t y_count = 0, x_count = 0;
          double x, y, z0, z1, prev_x = 999.0, prev_y = 999.0;
          uint8_t first_x = NVTrue;

          while (ngets (sep_string, sizeof (sep_string), sep_fp) != NULL)
            {
              sscanf (sep_string, "%lf,%lf,%lf,%lf", &y, &x, &z0, &z1);

              if (first_x)
                {
                  bvc.swCornerX = x;
                  first_x = NVFalse;
                }

              if (prev_y != y)
                {
                  y_count++;
                  bvc.nodeSpacingY = prev_y - y;
                }

              if (prev_x != x) x_count++;

              prev_y = y;
              prev_x = x;
            }

          fseek (sep_fp, pos, SEEK_SET);

          x_count /= y_count;

          opt_data_sep.opt[Surface_Correction].ncols = x_count;
          opt_data_sep.opt[Surface_Correction].nrows = y_count;
          bvc.nodeSpacingX = x - prev_x;
          bvc.swCornerY = y;
        }

      err = bagWriteCorrectorDefinition (bagHandle, &bvc);      
      if (err != BAG_SUCCESS)
        {
          string = tr ("Could not write corrector definition");

          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

          QMessageBox::warning (this, tr ("pfmBag Error"), string);

          exit (-1);
        }


      err = bagCreateCorrectorDataset (bagHandle, &opt_data_sep, 2, BAG_SURFACE_GRID_EXTENTS);      
      if (err != BAG_SUCCESS)
        {
          string = tr ("Error creating corrector dataset");

          u8 *errstr;

          if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

          QMessageBox::warning (this, tr ("pfmBag Error"), string);

          exit (-1);
        }

      bagVerticalCorrector *sep_depth = (bagVerticalCorrector *) calloc (opt_data_sep.def.ncols, sizeof (bagVerticalCorrector));
      if (sep_depth == NULL)
        {
          string = tr ("Error allocating sep_depth : %1").arg (strerror (errno));
          QMessageBox::critical (this, tr ("pfmBag Error"), string);
          exit (-1);
        }

      for (uint32_t i = 0 ; i < opt_data_sep.def.nrows ; i++)
        {
          NV_I32_COORD2 coord;

          coord.y = i;

          for (uint32_t j = 0 ; j < opt_data_sep.def.ncols ; j++)
            {
              coord.x = j;

              if (sep_fp)
                {
                  ngets (sep_string, sizeof (sep_string), sep_fp);

                  sscanf (sep_string, "%lf,%lf,%f,%f", &sep_depth[j].y, &sep_depth[j].x, &sep_depth[j].z[0], &sep_depth[j].z[1]);
                }
              else
                {
                  chrtr2_read_record (sep_handle, coord, &sep_record);

                  chrtr2_get_lat_lon (sep_handle, &sep_depth[j].y, &sep_depth[j].x, coord);


                  //  SABER uses the opposite terminology for Z0 and Z1 from what CHRTR2 uses so we'll flip Z0 and Z1.

                  sep_depth[j].z[0] = sep_record.z1;
                  sep_depth[j].z[1] = sep_record.z0;
                }


                //  If we're making a UTM projected BAG, convert positions to UTM.

              if (system.coordSys == UTM)
                {
                  double x = sep_depth[j].x * NV_DEG_TO_RAD;
                  double y = sep_depth[j].y * NV_DEG_TO_RAD;
                  pj_status = pj_transform (pfm_proj, bag_proj, 1, 1, &x, &y, NULL);
                  if (pj_status)
                    {
                      QString err_str = tr ("Proj.4 transform error at line %1 in %2\nError: %3\nInputs: %L4, %L5\nOutputs: %L6, %L7").arg
                        (__LINE__).arg (__FUNCTION__).arg (pj_strerrno (pj_status)).arg (sep_depth[j].x, 0, 'f', 11).arg (sep_depth[j].y, 0, 'f', 11).arg
                        (x, 0, 'f', 11).arg (y, 0, 'f', 11);
                      QMessageBox::critical (this, tr ("pfmBag Error"), err_str);
                      exit (-1);
                    }
                  sep_depth[j].x = x;
                  sep_depth[j].x = y;
                }
            }

          err = bagWriteRow (bagHandle, i, 0, opt_data_sep.def.ncols - 1, Surface_Correction, (void *) sep_depth);
        }

      if (sep_fp)
        {
          fclose (sep_fp);
        }
      else
        {
          chrtr2_close_file (sep_handle);
        }

      free (sep_depth);

      bagWriteCorrectorVerticalDatum (bagHandle, 1, (u8 *) "Mean lower low water = Vertical Datum");
    }


  //  IMPORTANT NOTE: bagFileClose will free the xmlBuffer.

  if ((err = bagFileClose (bagHandle)) != BAG_SUCCESS)
    {
      string = tr ("Error closing BAG file");

      u8 *errstr;

      if (bagGetErrorString (err, &errstr) == BAG_SUCCESS) string += (QString (" : ") + QString ((char *) errstr));

      QMessageBox::warning (this, tr ("pfmBag Error"), string);

      exit (-1);
    }


  //  Free the metadata

  bagFreeMetadata (&bag_metadata); 


  button (QWizard::FinishButton)->setEnabled (true);
  button (QWizard::CancelButton)->setEnabled (false);


  QApplication::restoreOverrideCursor ();


  checkList->addItem (" ");
  cur = new QListWidgetItem (tr ("Conversion complete, press Finish to exit."));

  checkList->addItem (cur);

  checkList->setCurrentItem (cur);

  checkList->scrollToItem (cur);

  close_pfm_file (pfm_handle);
}



//  Get the users defaults.

void pfmBag::envin (OPTIONS *options)
{
  //  We need to get the font from the global settings.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  options->font = QApplication::font ();

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  QString defaultFont = options->font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  options->font.fromString (fontString);


  settings2.endGroup ();


  double saved_version = 2.0;


  // Set defaults so that if keys don't exist the parameters are defined

  options->surface = CUBE_SURFACE;
  options->uncertainty = TPE_UNCERT;
  options->enhanced = NVTrue;
  options->units = 0;
  options->elev_off = 0.0;
  options->depth_cor = 0;
  options->v_datum = 53;
  options->non_radius = 5.0;
  options->source = QString ("Naval Oceanographic Office");
  options->classification = 0;
  options->authority = 0;
  options->distStatement = QString ("Approved for public release; distribution is unlimited.");
  options->pi_name = "";
  options->pi_title = "";
  options->poc_name = "";
  options->input_dir = ".";
  options->output_dir = ".";
  options->area_dir = ".";
  options->sep_dir = ".";
  options->pfm_wkt = "";
  options->bag_wkt = "";
  options->window_x = 0;
  options->window_y = 0;
  options->window_width = 900;
  options->window_height = 500;


#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/pfmBag.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/pfmBag.ini";
#endif


  QSettings settings (ini_file, QSettings::IniFormat);

  settings.beginGroup (QString ("pfmBag"));

  saved_version = settings.value (QString ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->surface = settings.value (QString ("surface"), options->surface).toInt ();

  options->uncertainty = settings.value (QString ("uncertainty"), options->uncertainty).toInt ();

  options->enhanced = settings.value (QString ("enhanced surface flag"), options->enhanced).toBool ();

  options->units = settings.value (QString ("units"), options->units).toInt ();

  options->elev_off = settings.value (QString ("elevation offset"), options->elev_off).toFloat ();

  options->depth_cor = settings.value (QString ("depth correction"), options->depth_cor).toInt ();

  options->v_datum = settings.value (QString ("vertical datum"), options->v_datum).toInt ();

  options->non_radius = settings.value (QString ("non-pfmFeature radius"), options->non_radius).toFloat ();


  options->source = settings.value (QString ("data source"), options->source).toString ();

  options->classification = settings.value (QString ("classification"), options->classification).toInt ();

  options->authority = settings.value (QString ("declassification authority"), options->authority).toInt ();


  options->distStatement = settings.value (QString ("distribution statement"), options->distStatement).toString ();


  options->pi_name = settings.value (QString ("PI name"), options->pi_name).toString ();

  options->pi_title = settings.value (QString ("PI title"), options->pi_title).toString ();

  options->poc_name = settings.value (QString ("POC name"), options->poc_name).toString ();

  options->pfm_wkt = settings.value (QString ("PFM WKT"), options->pfm_wkt).toString ();
  options->bag_wkt = settings.value (QString ("BAG WKT"), options->bag_wkt).toString ();

  options->input_dir = settings.value (QString ("input directory"), options->input_dir).toString ();
  options->output_dir = settings.value (QString ("output directory"), options->output_dir).toString ();
  options->area_dir = settings.value (QString ("area directory"), options->area_dir).toString ();
  options->sep_dir = settings.value (QString ("separation directory"), options->sep_dir).toString ();

  options->window_width = settings.value (QString ("width"), options->window_width).toInt ();
  options->window_height = settings.value (QString ("height"), options->window_height).toInt ();
  options->window_x = settings.value (QString ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (QString ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void pfmBag::envout (OPTIONS *options)
{
#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/pfmBag.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/pfmBag.ini";
#endif


  QSettings settings (ini_file, QSettings::IniFormat);

  settings.beginGroup (QString ("pfmBag"));


  settings.setValue (QString ("settings version"), settings_version);


  settings.setValue (QString ("surface"), options->surface);

  settings.setValue (QString ("uncertainty"), options->uncertainty);

  settings.setValue (QString ("enhanced surface flag"), options->enhanced);

  settings.setValue (QString ("units"), options->units);

  settings.setValue (QString ("elevation offset"), options->elev_off);

  settings.setValue (QString ("depth correction"), options->depth_cor);

  settings.setValue (QString ("vertical datum"), options->v_datum);

  settings.setValue (QString ("non-pfmFeature radius"), options->non_radius);


  settings.setValue (QString ("data source"), options->source);

  settings.setValue (QString ("classification"), options->classification);

  settings.setValue (QString ("declassification authority"), options->authority);


  settings.setValue (QString ("distribution statement"), options->distStatement);


  settings.setValue (QString ("PI name"), options->pi_name);

  settings.setValue (QString ("PI title"), options->pi_title);

  settings.setValue (QString ("POC name"), options->poc_name);

  settings.setValue (QString ("PFM WKT"), options->pfm_wkt);
  settings.setValue (QString ("BAG WKT"), options->bag_wkt);

  settings.setValue (QString ("input directory"), options->input_dir);
  settings.setValue (QString ("output directory"), options->output_dir);
  settings.setValue (QString ("area directory"), options->area_dir);
  settings.setValue (QString ("separation directory"), options->sep_dir);

  settings.setValue (QString ("width"), options->window_width);
  settings.setValue (QString ("height"), options->window_height);
  settings.setValue (QString ("x position"), options->window_x);
  settings.setValue (QString ("y position"), options->window_y);

  settings.endGroup ();
}
