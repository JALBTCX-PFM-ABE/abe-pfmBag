
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



#include "datumPage.hpp"
#include "datumPageHelp.hpp"

datumPage::datumPage (QWidget *pa, OPTIONS *op):
  QWizardPage (pa)
{
  parent = pa;
  options = op;
  io_flag = 0;


  setTitle (tr ("Coordinate Reference System parameters"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmBagWatermark.png"));


  units = new QComboBox (this);
  units->setWhatsThis (unitsText);
  units->setEditable (false);
  units->addItem (tr ("Meters"));
  units->addItem (tr ("Feet"));
  units->addItem (tr ("Fathoms"));
  units->setCurrentIndex (options->units);


  elevOff = new QDoubleSpinBox (this);
  elevOff->setDecimals (3);
  elevOff->setRange (-100.0, 100.0);
  elevOff->setSingleStep (1.0);
  elevOff->setValue (options->elev_off);
  elevOff->setWrapping (true);
  elevOff->setToolTip (tr ("Set an offset to be applied to the BAG elevation data"));
  elevOff->setWhatsThis (elevOffText);


  depthCor = new QComboBox (this);
  depthCor->setWhatsThis (depthCorText);
  depthCor->setEditable (false);
  depthCor->addItem (tr ("Corrected depth"));
  depthCor->addItem (tr ("Uncorrected depth @ 1500 m/s"));
  depthCor->addItem (tr ("Uncorrected depth @ 4800 ft/s"));
  depthCor->addItem (tr ("Uncorrected depth @ 800 fm/s"));
  depthCor->addItem (tr ("Mixed corrections"));
  depthCor->setCurrentIndex (options->depth_cor);


  pfmCrs = new QPushButton ("None", this);
  pfmCrs->setToolTip (tr ("Select the horizontal Coordinate Reference System (CRS) for the input PFM file"));
  pfmCrs->setWhatsThis (pfmCrsText);
  connect (pfmCrs, SIGNAL (clicked ()), this, SLOT (slotPfmCrsClicked ()));


  bagCrs = new QPushButton ("None", this);
  bagCrs->setToolTip (tr ("Select the horizontal Coordinate Reference System (CRS) for the output BAG file"));
  bagCrs->setWhatsThis (bagCrsText);
  connect (bagCrs, SIGNAL (clicked ()), this, SLOT (slotBagCrsClicked ()));


  vDatum = new QComboBox (this);
  vDatum->setToolTip (tr ("Select the vertical datum for the output BAG file"));
  vDatum->setWhatsThis (vDatumText);
  vDatum->setEditable (false);
  for (int32_t i = 0 ; i < options->v_datum_count ; i++)
    {
      if (options->v_datums[i].active) vDatum->addItem (options->v_datums[i].name, i);
    }
  vDatum->setCurrentIndex (vDatum->findData (options->v_datum));

  connect (vDatum, SIGNAL (activated (int)), this, SLOT (slotVDatumActivated (int)));


  source = new QLineEdit (this);
  source->setWhatsThis (sourceText);
  source->setText (options->source);


  QFormLayout *formLayout = new QFormLayout;


  formLayout->addRow (tr ("&Units:"), units);
  formLayout->addRow (tr ("&Elevation offset:"), elevOff);
  formLayout->addRow (tr ("&Depth correction:"), depthCor);
  formLayout->addRow (tr ("&PFM Horizontal CRS:"), pfmCrs);
  formLayout->addRow (tr ("&BAG Horizontal CRS:"), bagCrs);
  formLayout->addRow (tr ("&BAG Vertical datum:"), vDatum);


  formLayout->addRow (tr ("Data &source:"), source);
  setLayout (formLayout);


  registerField ("units", units);
  registerField ("elevOff", elevOff);
  registerField ("depthCor", depthCor);
  registerField ("source", source);
}



void 
datumPage::setFields (OPTIONS *op)
{
  options = op;


  //  Open the PFM to see if it has WKT.

  PFM_OPEN_ARGS open_args;
  strcpy (open_args.list_path, options->pfm_file_name.toLatin1 ());
  open_args.checkpoint = 0;
  int32_t pfm_handle = open_existing_pfm_file (&open_args);
  if (pfm_handle < 0) pfm_error_exit (pfm_error);


  //  If the PFM file was built from CZMIL, LAS, or BAG files (and hopefully other data types in the future), the WKT should aleady be in the PFM header.
  //  Check for it here.

  QString pfm_wkt = QString (open_args.head.proj_data.wkt);
  close_pfm_file (pfm_handle);
  use_pfm_wkt = NVFalse;
  QString pfm_wkt_name = "None";


  //  If the PFM WKT is a composite CRS strip the vertical section.

  OGRSpatialReference pfmSRS;
  char wkt[8192];
  strcpy (wkt, pfm_wkt.toLatin1 ());
  char *ppsz, *ptr_wkt = wkt;

  if (pfmSRS.importFromWkt (&ptr_wkt) == OGRERR_NONE)
    {
      //  Make sure it's either projected or geographic...

      if (pfmSRS.IsProjected () || pfmSRS.IsGeographic ())
        {
          //  If it's compound, break out the horizontal part...

          if (pfmSRS.IsCompound ())
            {
              pfmSRS.StripVertical ();

              pfmSRS.exportToWkt (&ppsz);

              pfm_wkt = QString (ppsz);

              OGRFree (ppsz);
            }
        }
    }


  if (pfm_wkt.contains ("GEOGCS"))
    {
      if (pfm_wkt.contains ("PROJCS"))
        {
          pfm_wkt_name = pfm_wkt.section ("PROJCS[", 1, 1).section (',', 0, 0);
        }
      else
        {
          pfm_wkt_name = pfm_wkt.section ("GEOGCS[", 1, 1).section (',', 0, 0);
        }
    }


  pfmCrs->setText (pfm_wkt_name);


  if (pfmCrs->text () != "None")
    {
      //  Make the WKT human readable

      OGRSpatialReference SRS;
      char wkt[8192], pretty[8192];
      strcpy (wkt, options->pfm_wkt.toLatin1 ());
      char *ppszPretty, *ptr_wkt = wkt;

      SRS.importFromWkt (&ptr_wkt);

      SRS.exportToPrettyWkt (&ppszPretty);

      strcpy (pretty, ppszPretty);
      OGRFree (ppszPretty);

      QString prettyToolTip = QString (pretty);

      pfmCrs->setToolTip (tr ("Select the horizontal Coordinate Reference System for the input PFM file.  Current PFM CRS:<br><br>%1").arg (prettyToolTip));
    }


  //  If the BAG WKT is a composite CRS strip the vertical section.

  OGRSpatialReference bagSRS;
  strcpy (wkt, options->bag_wkt.toLatin1 ());
  ptr_wkt = wkt;

  if (bagSRS.importFromWkt (&ptr_wkt) == OGRERR_NONE)
    {
      //  Make sure it's either projected or geographic...

      if (bagSRS.IsProjected () || bagSRS.IsGeographic ())
        {
          //  If it's compound, break out the horizontal part...

          if (bagSRS.IsCompound ())
            {
              bagSRS.StripVertical ();

              bagSRS.exportToWkt (&ppsz);

              options->bag_wkt = QString (ppsz);

              OGRFree (ppsz);
            }
        }
    }


  QString bag_wkt_name = "None";

  if (options->bag_wkt.contains ("GEOGCS"))
    {
      if (options->bag_wkt.contains ("PROJCS"))
        {
          bag_wkt_name = options->bag_wkt.section ("PROJCS[", 1, 1).section (',', 0, 0);
        }
      else
        {
          bag_wkt_name = options->bag_wkt.section ("GEOGCS[", 1, 1).section (',', 0, 0);
        }
    }

  bagCrs->setText (bag_wkt_name);


  if (bagCrs->text () != "None")
    {
      //  Make the WKT human readable

      OGRSpatialReference SRS;
      char wkt[8192], pretty[8192];
      strcpy (wkt, options->bag_wkt.toLatin1 ());
      char *ppszPretty, *ptr_wkt = wkt;

      SRS.importFromWkt (&ptr_wkt);

      SRS.exportToPrettyWkt (&ppszPretty);

      strcpy (pretty, ppszPretty);
      OGRFree (ppszPretty);

      QString prettyToolTip = QString (pretty);

      bagCrs->setToolTip (tr ("Select the horizontal Coordinate Reference System for the output BAG file.  Current BAG CRS:<br><br>%1").arg (prettyToolTip));
    }
}



bool 
datumPage::validatePage ()
{
  if (pfmCrs->text () == "None")
    {
      QMessageBox::critical (this, "pfmBag", tr ("You must select a Coordinate Reference System for the input PFM file to continue!"));

      return (false);
    }

  if (bagCrs->text () == "None")
    {
      QMessageBox::critical (this, "pfmBag", tr ("You must select a Coordinate Reference System for the output BAG file to continue!"));

      return (false);
    }

  return (true);
}



void 
datumPage::slotPfmCrsClicked ()
{
  if (use_pfm_wkt)
    {
      int32_t ret = QMessageBox::Yes;
      ret = QMessageBox::information (this, "pfmBag", pfmWktText, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

      if (ret == QMessageBox::No) return;
    }


  io_flag = 0;
  wkt_dialog = new wktDialog (parent, options, io_flag);
  wkt_dialog->setModal (true);
  connect (wkt_dialog, SIGNAL (finished (int)), this, SLOT (slotWktDialogFinished (int)));
}



void 
datumPage::slotBagCrsClicked ()
{
  io_flag = 1;
  wkt_dialog = new wktDialog (parent, options, io_flag);
  wkt_dialog->setModal (true);
  connect (wkt_dialog, SIGNAL (finished (int)), this, SLOT (slotWktDialogFinished (int)));
}



void 
datumPage::slotWktDialogFinished (int result)
{
  if (result)
    {
      if (io_flag)
        {
          QString bag_wkt_name;

          if (options->bag_wkt.contains ("GEOGCS"))
            {
              if (options->bag_wkt.contains ("PROJCS"))
                {
                  bag_wkt_name = options->bag_wkt.section ("PROJCS[", 1, 1).section (',', 0, 0);
                }
              else
                {
                  bag_wkt_name = options->bag_wkt.section ("GEOGCS[", 1, 1).section (',', 0, 0);
                }
            }

          bagCrs->setText (bag_wkt_name);


          //  Make the WKT human readable

          OGRSpatialReference SRS;
          char wkt[8192], pretty[8192];
          strcpy (wkt, options->bag_wkt.toLatin1 ());
          char *ppszPretty, *ptr_wkt = wkt;

          SRS.importFromWkt (&ptr_wkt);

          SRS.exportToPrettyWkt (&ppszPretty);

          strcpy (pretty, ppszPretty);
          OGRFree (ppszPretty);

          QString prettyToolTip = QString (pretty);

          bagCrs->setToolTip (tr ("Select the Coordinate Reference System for the output BAG file.  Current BAG CRS:<br><br>%1").arg (prettyToolTip));
        }
      else
        {
          if (options->pfm_wkt.contains ("PROJCS"))
            {
              QMessageBox::critical (this, "pfmBag", tr ("You cannot use a projected Coordinate Reference System for the input PFM!"));
              return;
            }


          QString pfm_wkt_name;

          if (options->pfm_wkt.contains ("GEOGCS"))
            {
              pfm_wkt_name = options->pfm_wkt.section ("GEOGCS[", 1, 1).section (',', 0, 0);
            }

          pfmCrs->setText (pfm_wkt_name);


          //  Make the WKT human readable

          OGRSpatialReference SRS;
          char wkt[8192], pretty[8192];
          strcpy (wkt, options->pfm_wkt.toLatin1 ());
          char *ppszPretty, *ptr_wkt = wkt;

          SRS.importFromWkt (&ptr_wkt);

          SRS.exportToPrettyWkt (&ppszPretty);

          strcpy (pretty, ppszPretty);
          OGRFree (ppszPretty);

          QString prettyToolTip = QString (pretty);

          pfmCrs->setToolTip (tr ("Select the Coordinate Reference System for the input PFM file.  Current PFM CRS:<br><br>%1").arg (prettyToolTip));
        }
    }
}



void 
datumPage::slotVDatumActivated (int index)
{
  int32_t save_v_datum = options->v_datum;


  //  We need the index into the array, not the combo box.

  options->v_datum = vDatum->itemData (index).toInt ();


  //  Ask for user input for "OTHER".

  if (options->v_datums[options->v_datum].abbrev == "OTHER")
    {
      bool ok;

      QString text = QInputDialog::getText (this, "pfmBag", tr ("Enter a brief description of the vertical datum (e.g. NAVD88 corrected to MLLW):"),
                                            QLineEdit::Normal, QString::null, &ok);

      if (ok && !text.isEmpty ())
        {
          options->v_datums[options->v_datum].name = text;
          vDatum->setItemText (index, text);
        }
      else
        {
          options->v_datum = save_v_datum;
        }
    }
}
