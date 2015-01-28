#include "zoomdialog.h"

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

ZoomDialog::ZoomDialog(QWidget *parent)
{
  m_toolwidget = static_cast<ToolWidget*>(parent);
  //setAttribute(Qt::WA_StaticContents);
  this->setFixedSize(300, 200);
  pushButton_Apply = new QPushButton("Apply", this);
  connect(pushButton_Apply, SIGNAL(clicked()), this, SLOT(apply()));

  inputline_r = new QDoubleSpinBox;
  inputline_theta = new QDoubleSpinBox;
  inputline_phi = new QDoubleSpinBox;

  QLabel *text_r = new QLabel("r");
  text_r->setFont( QFont("Times", 10, QFont::Bold) );
  text_r->setAlignment( Qt::AlignCenter);

  QLabel *text_theta = new QLabel("theta");
  text_theta->setFont( QFont("Times", 10, QFont::Bold) );
  text_theta->setAlignment( Qt::AlignCenter);

  QLabel *text_phi = new QLabel("phi");
  text_phi->setFont( QFont("Times", 10, QFont::Bold) );
  text_phi->setAlignment( Qt::AlignCenter);

  QGridLayout *DialogLayout = new QGridLayout();
  DialogLayout->addWidget( text_r, 0, 0 );
  DialogLayout->addWidget( text_theta, 0, 1 );
  DialogLayout->addWidget( text_phi, 2, 0 );
  DialogLayout->addWidget( inputline_r, 1, 0 );
  DialogLayout->addWidget( inputline_theta, 1, 1 );
  DialogLayout->addWidget( inputline_phi, 3, 0 );
  DialogLayout->addWidget( pushButton_Apply, 0, 2 );
  
  inputline_r->setSingleStep( 0.1 );
  inputline_theta->setSingleStep( 5 );
  inputline_phi->setSingleStep( 5 );

  inputline_r->setDecimals(1);
  inputline_theta->setDecimals(0);
  inputline_phi->setDecimals(0);
  inputline_theta->setMaximum(360);
  inputline_phi->setMaximum(360);
  connect(inputline_r, SIGNAL(valueChanged(double)),this,SLOT(setR(double)) );
  connect(inputline_theta, SIGNAL(valueChanged(double)),this,SLOT(setTheta(double)) );
  connect(inputline_phi, SIGNAL(valueChanged(double)),this,SLOT(setPhi(double)) );
  this->setLayout(DialogLayout);
}


ZoomDialog::~ZoomDialog()
{
}

void ZoomDialog::apply()
{
  m_toolwidget->public_zoom(r,theta,phi);
}

void ZoomDialog::setRange(float input_min1, float input_max1, float input_min2, float input_max2)
{
}

void ZoomDialog::setR(double inputr)
{
  r = inputr;
  return;
}

void ZoomDialog::setTheta(double inputtheta)
{
  theta=inputtheta;
  return;
}

void ZoomDialog::setPhi(double inputphi)
{
  phi=inputphi;
  return;
}
