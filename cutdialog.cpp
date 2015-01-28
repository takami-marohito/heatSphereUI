#include "cutdialog.h"

#include <kvs/qt/Screen>
#include <QListWidget>
#include <QLabel>

#include "toolwidget.h"
#include "renderingwidget.h"
#include "TFwidget.h"

#define ERASE 3
#define CRAMP 2
#define FILL 1
#define PEN 0

CutDialog::CutDialog(QWidget *parent)
{
  m_toolwidget = static_cast<ToolWidget*>(parent);
  //setAttribute(Qt::WA_StaticContents);
  this->setFixedSize(300, 200);
  pushButton_Apply = new QPushButton("Apply", this);
  connect(pushButton_Apply, SIGNAL(clicked()), this, SLOT(apply()));

  inputline_sei_min = new QDoubleSpinBox;
  inputline_gt_min = new QDoubleSpinBox;
  inputline_sei_max = new QDoubleSpinBox;
  inputline_gt_max = new QDoubleSpinBox;

  QLabel *text_sei = new QLabel("SecondInvariant");
  text_sei->setFont( QFont("Times", 10, QFont::Bold) );
  text_sei->setAlignment( Qt::AlignCenter);

  QLabel *text_gt = new QLabel("GradT");
  text_gt->setFont( QFont("Times", 10, QFont::Bold) );
  text_gt->setAlignment( Qt::AlignCenter);


  QGridLayout *DialogLayout = new QGridLayout();
  DialogLayout->addWidget( text_sei, 0, 0 );
  DialogLayout->addWidget( text_gt, 0, 1 );
  DialogLayout->addWidget( inputline_sei_max, 1, 0 );
  DialogLayout->addWidget( inputline_gt_max, 1, 1 );
  DialogLayout->addWidget( inputline_sei_min, 2, 0 );
  DialogLayout->addWidget( inputline_gt_min, 2, 1 );
  DialogLayout->addWidget( pushButton_Apply, 3, 0, 1, 2 );
  
  inputline_sei_min->setSingleStep( 100 );
  inputline_gt_min->setSingleStep( 100 );

  inputline_sei_min->setSuffix(" min");
  inputline_sei_max->setSuffix(" max");

  inputline_sei_min->setDecimals(0);
  inputline_sei_min->setMaximum(100000);
  inputline_gt_min->setDecimals(0);
  inputline_gt_min->setMaximum(300000);

  inputline_sei_max->setSingleStep( 100 );
  inputline_gt_max->setSingleStep( 100 );

  inputline_sei_max->setDecimals(0);
  inputline_sei_max->setMaximum(100000);
  inputline_gt_max->setDecimals(0);
  inputline_gt_max->setMaximum(300000);

  connect(inputline_sei_min, SIGNAL(valueChanged(double)),this,SLOT(setsei_min(double)) );
  connect(inputline_gt_min, SIGNAL(valueChanged(double)),this,SLOT(setgt_min(double)) );
  connect(inputline_sei_max, SIGNAL(valueChanged(double)),this,SLOT(setsei_max(double)) );
  connect(inputline_gt_max, SIGNAL(valueChanged(double)),this,SLOT(setgt_max(double)) );

  this->setLayout(DialogLayout);
}


CutDialog::~CutDialog()
{
}

void CutDialog::apply()
{
  m_toolwidget->public_cut(cut_sei_min, cut_sei_max, cut_gt_min, cut_gt_max);
}


void CutDialog::setsei_min(double inputsei_min)
{
  cut_sei_min = inputsei_min;
  return;
}

void CutDialog::setgt_min(double inputgt_min)
{
  cut_gt_min=inputgt_min;
  return;
}

void CutDialog::setsei_max(double inputsei_max)
{
  cut_sei_max = inputsei_max;
  return;
}

void CutDialog::setgt_max(double inputgt_max)
{
  cut_gt_max=inputgt_max;
  return;
}

