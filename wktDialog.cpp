
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


#include "wktDialog.hpp"


wktDialog::wktDialog (QWidget *pa, OPTIONS *op, uint8_t io):
  QDialog (pa, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  parent = pa;
  options = op;
  io_flag = io;
  recent_id = -1;


  //  We need to get any recent WKT values from the globalABE settings file.  This file is used to store things that
  //  may be used by multiple ABE programs.

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("globalABE");


  for (int32_t i = 0 ; i < 10 ; i++)
    {
      options->wktString[i] = "";

      QString name = tr ("Recent WKT %1").arg (i);
      options->wktString[i] = settings.value (name, options->wktString[i]).toString ();
    }

  settings.endGroup ();


  //  We always want to have WGS84 and NAD83.

  QString wgs84 = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]";
  QString nad83 = "GEOGCS[\"NAD83\",DATUM[\"North_American_Datum_1983\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"6269\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4269\"]]";


  //  These would be the COMPD_CS versions - combined with the vertical CRS.

  //QString wgs84 = "COMPD_CS[\"WGS84 with WGS84E Z\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]],VERT_CS[\"WGS84E Z in metres\",VERT_DATUM[\"Ellipsoid\",2002],UNIT[\"metre\",1],AXIS[\"Z\",UP]]]";
  //QString nad83 = "COMPD_CS[\"NAD83 with NAVD88 Z\",GEOGCS[\"NAD83\",DATUM[\"North_American_Datum_1983\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"6269\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4269\"]],VERT_CS[\"NAVD88 Z in metres\",VERT_DATUM[\"North American Vertical Datum 1988\",2005,AUTHORITY[\"EPSG\",\"5103\"]],AXIS[\"Gravity-related height\",UP],UNIT[\"metre\",1.0,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"5703\"]]]";


  uint8_t wflag = NVFalse;
  uint8_t nflag = NVFalse;

  for (int32_t i = 0 ; i < 10 ; i++)
    {
      if (options->wktString[i] == wgs84) wflag = NVTrue;
      if (options->wktString[i] == nad83) nflag = NVTrue;
    }

  if (!nflag)
    {
      //  Add the NAD83 WKT string to the top of the recently used list.

      for (int32_t i = 8 ; i >= 0 ; i--) options->wktString[i + 1] = options->wktString[i];
      options->wktString[0] = nad83;
    }

  if (!wflag)
    {
      //  Add the WGS84 WKT string to the top of the recently used list.

      for (int32_t i = 8 ; i >= 0 ; i--) options->wktString[i + 1] = options->wktString[i];
      options->wktString[0] = wgs84;
    }


  //  If we added either, save the settings.

  if (nflag || wflag)
    {
#ifdef NVWIN3X
      QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
      QString ini_file = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

      QSettings settings (ini_file, QSettings::IniFormat);
      settings.beginGroup ("globalABE");


      for (int32_t i = 0 ; i < 10 ; i++)
        {
          QString name = tr ("Recent WKT %1").arg (i);
          settings.setValue (name, options->wktString[i]);
        }

      settings.endGroup ();
    }


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QString msg;

  if (io_flag)
    {
      msg = tr ("This dialog will allow you to enter Well-known Text (WKT) to be used to convert the input geographic PFM data to be placed in the BAG file.  "
                "WKT for many different areas is available on <a href=\"http://spatialreference.org/\">spatialreference.org</a> (for example, "
                "<a href=\"http://spatialreference.org/ref/epsg/26916/\">EPSG:26916</a>).  Only GEOGCS or PROJCS (UTM) WKT is supported.<br><br>"
                "Simply copy the <b>OGC WKT</b> that best matches the data in the input PFM file and paste it into the text box below.");
    }
  else
    {
      msg = tr ("The input PFM did not have a Well-known Text (WKT) coordinate reference system (CRS) definition in the header.  You also did not "
                "select either WGS84 or NAD83.  This dialog will allow you to enter Well-known Text (WKT) to be used to convert the positions.  "
                "WKT for many different areas is available on <a href=\"http://spatialreference.org/\">spatialreference.org</a> (for example, "
                "<a href=\"http://spatialreference.org/ref/epsg/4326/\">EPSG:4326</a>).  Only GEOGCS WKT is supported.<br><br>"
                "Simply copy the <b>OGC WKT</b> that best matches the data in the input PFM file and paste it into the text box below.");
    }

  QLabel *wktLabel = new QLabel (this);
  wktLabel->setWordWrap (true);
  wktLabel->setOpenExternalLinks (true);
  wktLabel->setTextFormat (Qt::RichText);

  wktLabel->setText (msg);


  vbox->addWidget (wktLabel);


  QHBoxLayout *wktLayout = new QHBoxLayout (0);
  vbox->addLayout (wktLayout);

  recentWKT = new QComboBox (this);
  recentWKT->setToolTip (tr ("Select a previously used WKT string (hover to see WKT in tool tip)"));
  recentWKT->setWhatsThis (tr ("Select a previously used WKT string (hover to see WKT in tool tip)"));
  recentWKT->setEditable (false);
  for (int32_t i = 0 ; i < 10 ; i++)
    {
      if (!options->wktString[i].isEmpty ())
        {
          //  Make the WKT human readable

          OGRSpatialReference SRS;
          char wkt[8192], pretty[8192];
          strcpy (wkt, options->wktString[i].toLatin1 ());
          char *ppszPretty, *ptr_wkt = wkt;

          SRS.importFromWkt (&ptr_wkt);

          SRS.exportToPrettyWkt (&ppszPretty);

          strcpy (pretty, ppszPretty);
          OGRFree (ppszPretty);

          QString prettyToolTip = QString (pretty);


          QString tmp;

          if (options->wktString[i].contains ("PROJCS"))
            {
              tmp = options->wktString[i].section ("PROJCS[", 1, 1).section (',', 0, 0);
            }
          else
            {
              tmp = options->wktString[i].section ("GEOGCS[", 1, 1).section (',', 0, 0);
            }

          recentWKT->addItem (tmp);
          recentWKT->setItemData (i, prettyToolTip, Qt::ToolTipRole);
        }
    }
  connect (recentWKT, SIGNAL (activated (int)), this, SLOT (slotRecentWKTActivated (int)));
  wktLayout->addWidget (recentWKT);



  wktText = new QLineEdit (this);
  if (io_flag)
    {
      wktText->setToolTip (tr ("Enter the Well-known Text (WKT) for the BAG output file"));
      wktText->setWhatsThis (tr ("Enter the Well-known Text (WKT) coordinate system definition for the BAG output file"));

      if (options->bag_wkt.isEmpty ())
        {
          wktText->setPlaceholderText (tr ("Well-known Text (WKT)"));
        }
      else
        {
          wktText->setText (options->bag_wkt);
        }
    }
  else
    {
      wktText->setToolTip (tr ("Enter the Well-known Text (WKT) for the PFM input file"));
      wktText->setWhatsThis (tr ("Enter the Well-known Text (WKT) coordinate system definition for the PFM input file"));

      if (options->pfm_wkt.isEmpty ())
        {
          wktText->setPlaceholderText (tr ("Well-known Text (WKT)"));
        }
      else
        {
          wktText->setText (options->pfm_wkt);
        }
    }

  connect (wktText, SIGNAL (textEdited (QString)), this, SLOT (slotWktTextEdited (QString)));
  wktLayout->addWidget (wktText);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.png"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *rejectButton = new QPushButton (tr ("Reject"), this);
  rejectButton->setToolTip (tr ("Reject WKT text and close this dialog"));
  connect (rejectButton, SIGNAL (clicked ()), this, SLOT (slotRejectWKT ()));
  actions->addWidget (rejectButton);

  QPushButton *acceptButton = new QPushButton (tr ("Accept"), this);
  acceptButton->setToolTip (tr ("Accept the WKT text that has been entered and close this dialog"));
  connect (acceptButton, SIGNAL (clicked ()), this, SLOT (slotAcceptWKT ()));
  actions->addWidget (acceptButton);


  //  Make the dialog the same size and location as the QWizard.

  resize (parent->width (), parent->height ());
  move (parent->x (), parent->y ());


  show ();
}



wktDialog::~wktDialog ()
{
}



void 
wktDialog::slotRecentWKTActivated (int id)
{
  recent_id = id;

  wktText->setText (options->wktString[id]);
}



void 
wktDialog::slotWktTextEdited (QString text __attribute__ ((unused)))
{
  recent_id = -1;
}



void
wktDialog::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
wktDialog::slotAcceptWKT ()
{
  QString text = wktText->text ().simplified ();

  if (!text.isEmpty ())
    {
      if ((!text.startsWith ("GEOGCS")) && (!text.startsWith ("PROJCS")))
        {
          QMessageBox::warning (parent, tr ("%1 Well-known Text").arg (options->progname),
                                tr ("The WKT data entered does not appear to be correct and/or supported."));

          wktText->setText ("");
        }
      else
        {
          if (io_flag)
            {
              options->bag_wkt = text;
            }
          else
            {
              options->pfm_wkt = text;
            }



          //  Check to see if we used one of the recently used WKT strings.

          if (recent_id < 0)
            {
              //  Add the new WKT string to the top of the recently used list.

              for (int32_t i = 8 ; i >= 0 ; i--) options->wktString[i + 1] = options->wktString[i];
              options->wktString[0] = text;
            }
          else
            {
              //  If the user selected the first one we don't need to re-order.

              if (recent_id > 0)
                {
                  //  Re-order the recently used WKT strings.

                  for (int32_t i = recent_id - 1 ; i >= 0 ; i--) options->wktString[i + 1] = options->wktString[i];
                  options->wktString[0] = text;
                }
            }


          //  We need to save the WKT strings to the globalABE settings file.  This file is used to store things that
          //  may be used by multiple ABE programs.

#ifdef NVWIN3X
          QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
          QString ini_file = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

          QSettings settings (ini_file, QSettings::IniFormat);
          settings.beginGroup ("globalABE");


          for (int32_t i = 0 ; i < 10 ; i++)
            {
              QString name = tr ("Recent WKT %1").arg (i);
              settings.setValue (name, options->wktString[i]);
            }

          settings.endGroup ();


          accept ();
        }
    }
  else
    {
      QMessageBox::warning (parent, tr ("%1 Well-known Text").arg (options->progname), tr ("The WKT text input field is empty."));
    }
}



void 
wktDialog::slotRejectWKT ()
{
  reject ();
}
