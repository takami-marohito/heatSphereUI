#include "rthetaphi.h"

#include <kvs/qt/Screen>

#include <QSlider>
#include <QListWidget>

#include "toolwidget.h"

RthetaphiSlider::RthetaphiSlider(QWidget *parent)
{
  mode=0;
  m_toolwidget = static_cast<ToolWidget*>(parent);
  QLabel *rlabel = new QLabel(this);
  QLabel *thetalabel = new QLabel(this);
  QLabel *philabel = new QLabel(this);

  rlabel->setText("radius");
  thetalabel->setText("theta");
  philabel->setText("phi");

  slider_r = new QSlider(Qt::Horizontal, this);
  slider_r->setRange(1,60);
  slider_r->setSliderPosition(5);

  slider_theta = new QSlider(Qt::Horizontal, this);
  slider_theta->setRange(-120,120);
  slider_theta->setSliderPosition(0);

  slider_phi = new QSlider(Qt::Horizontal, this);
  slider_phi->setRange(-120,120);
  slider_phi->setSliderPosition(0);

  connect(slider_r, SIGNAL(valueChanged(int)), this, SLOT(ChangeSlider_r(int)));
  connect(slider_theta, SIGNAL(valueChanged(int)), this, SLOT(ChangeSlider_theta(int)));
  connect(slider_phi, SIGNAL(valueChanged(int)), this, SLOT(ChangeSlider_phi(int)));

  bapply = new QPushButton("Apply", this);
  connect(bapply, SIGNAL(clicked()), this, SLOT(apply()));

  QGridLayout *ToolLayout = new QGridLayout();
  ToolLayout->addWidget( slider_r, 0,0 );
  ToolLayout->addWidget( slider_theta, 1,0 );
  ToolLayout->addWidget( slider_phi, 2,0 );
  ToolLayout->addWidget( rlabel, 0,2 );
  ToolLayout->addWidget( thetalabel, 1,2 );
  ToolLayout->addWidget( philabel, 2,2 );
  ToolLayout->addWidget( bapply, 3,0 );
  this->setLayout(ToolLayout);
}

RthetaphiSlider::~RthetaphiSlider()
{
}

void RthetaphiSlider::ChangeSlider_r(int parameter)
{
    m_toolwidget->setR(parameter);
}

void RthetaphiSlider::ChangeSlider_theta(int parameter)
{
    m_toolwidget->setTheta(parameter);
}

void RthetaphiSlider::ChangeSlider_phi(int parameter)
{
    m_toolwidget->setPhi(parameter);
}

void RthetaphiSlider::apply()
{
  m_toolwidget->applyrthetaphi();
}
