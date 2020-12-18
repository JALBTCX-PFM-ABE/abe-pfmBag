
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



#include "classPage.hpp"
#include "classPageHelp.hpp"

classPage::classPage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Classification/Distribution Information"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmBagWatermark.png"));


  classification = new QComboBox (this);
  classification->setWhatsThis (classificationText);
  classification->setEditable (false);
  classification->addItem (tr ("Unclassified"));
  classification->addItem (tr ("Confidential"));
  classification->addItem (tr ("Secret"));
  classification->addItem (tr ("Top Secret"));
  classification->setCurrentIndex (options->classification);


  authority = new QComboBox (this);
  authority->setWhatsThis (authorityText);
  authority->setEditable (false);
  authority->addItem (tr ("N/A"));
  authority->addItem (tr ("Derived from: OPNAVINSTS5513.5B(23)"));
  authority->addItem (tr ("Derived from: OPNAVINSTS5513.5B(24)"));
  authority->addItem (tr ("Derived from: OPNAVINSTS5513.5B(27)"));
  authority->addItem (tr ("Derived from: OPNAVINSTS5513.5B(28)"));
  authority->setCurrentIndex (options->authority);


  //  Set the declass date to today plus 10 years

  options->declassDate = QDate::currentDate ().addYears (10);

  declassDate = new QDateEdit (options->declassDate, this);
  declassDate->setWhatsThis (declassDateText);
  declassDate->setDisplayFormat ("yyyy-MM-dd");
  declassDate->setMinimumDate (QDate (2008, 4, 30));
  declassDate->setCalendarPopup (true);


  //  Set the compile date to today.

  options->compDate = QDate::currentDate ();

  compDate = new QDateEdit (options->compDate, this);
  compDate->setWhatsThis (compDateText);
  compDate->setDisplayFormat ("yyyy-MM-dd");
  compDate->setMinimumDate (QDate (1978, 1, 3));
  compDate->setCalendarPopup (true);


  distBox = new QComboBox (this);
  distBox->setWhatsThis (distBoxText);
  distBox->setEditable (false);
  distBox->addItem (tr ("Distribution Statement A"));
  distBox->addItem (tr ("Distribution Statement B"));
  distBox->addItem (tr ("Distribution Statement C"));
  distBox->addItem (tr ("Distribution Statement D"));
  distBox->addItem (tr ("Distribution Statement E with OSP information"));
  distBox->addItem (tr ("Distribution Statement E without OSP information"));
  distBox->addItem (tr ("Distribution Statement F"));
  distBox->addItem (tr ("Distribution Statement X"));
  distBox->addItem (tr ("Distribution Statement Disclaimer"));
  distBox->addItem (tr ("Distribution Statement with export-controlled technical data"));
  distBox->setCurrentIndex (0);
  connect (distBox, SIGNAL (activated (int)), this, SLOT (slotDistBoxActivated (int)));


  distStatement = new QTextEdit (this);
  distStatement->setWhatsThis (distStatementText);
  distStatement->setLineWrapMode (QTextEdit::WidgetWidth);
  distStatement->append (options->distStatement);


  QFormLayout *formLayout = new QFormLayout;


  formLayout->addRow (tr ("&Classification:"), classification);
  formLayout->addRow (tr ("Classifying &authority:"), authority);
  formLayout->addRow (tr ("&Declassification date:"), declassDate);
  formLayout->addRow (tr ("&Extract/Compile date:"), compDate);
  formLayout->addRow (tr ("&Distribution statement templates:"), distBox);
  formLayout->addRow (tr ("&Distribution statement:"), distStatement);
  setLayout (formLayout);


  registerField ("classification", classification);
  registerField ("authority", authority);
  registerField ("declassDate", declassDate);
  registerField ("compDate", compDate);
  registerField ("distStatement", distStatement, "plainText");
}



void 
classPage::slotDistBoxActivated (int index)
{
  distStatement->clear ();

  switch (index)
    {
    default:
    case 0:
      distStatement->append (tr ("Approved for public release; distribution is unlimited."));
      break;

    case 1:
      distStatement->append (tr ("Distribution limited to U.S. Government agencies only; <fill in reason>, <date>.  "
				 "Other requests for this document must be referred to <insert originating command>."));
      break;

    case 2:
      distStatement->append (tr ("Distribution authorized to U.S. Government agencies and their contractors, <fill in reason>, <date>.  "
				 "Other requests for this document shall be referred to <insert originating command>."));
      break;

    case 3:
      distStatement->append (tr ("Distribution authorized to DOD and DOD contractors only; <fill in reason>, <date>.  "
				 "Other U.S. requests shall be referred to the Commanding Officer, Naval Oceanographic Office."));
      break;

    case 4:
      distStatement->append (tr ("Distribution authorized to DoD components only.  Other requests shall be directed to CNO N77."));
      break;

    case 5:
      distStatement->append (tr ("Distribution authorized to DoD components only; ADM/OPS (date).  Other requests for this "
				 "information shall be directed to Commander, Naval Meteorology and Oceanography Command."));
      break;

    case 6:
      distStatement->append (tr ("Further dissemination only as directed by <insert originating command>, <date>, or higher DOD "
				 "authority."));
      break;

    case 7:
      distStatement->append (tr ("Distribution authorized to U.S. Governement agencies and private individuals or enterprises "
				 "eligible to obtain export-controlled technical data in accordance with reference <m>.  Other "
				 "requests must be referred to <originating command>."));
      break;

    case 8:
      distStatement->append (tr ("The inclusion of names of any specific commercial product, commodity, or service in this "
				 "publication is for information purposes only and does not imply endorsement by the Navy or NAVOCEANO."));
      break;

    case 9:
      distStatement->append (tr ("WARNING - This document contains technical data whose export is restricted by the Arms Export "
				 "Control Act (Title 22, U.S.C. Sec. 2751 et seq.) or the Export Administration Act of 1979, as "
				 "amended, Title 50, U.S.C., App 2401, et seq.  Violations of these export laws are subject to "
				 "severe criminal penalties.  Disseminate in accordance with provisions of OPNAVINST 5510.161."));
      break;
    }
}
