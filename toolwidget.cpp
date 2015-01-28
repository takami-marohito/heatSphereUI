#include "toolwidget.h"

#include <kvs/qt/Screen>
#include <QListWidget>
#include <QLabel>

#include <algorithm>
#include <math.h>

#include "renderingwidget.h"
#include "TFwidget.h"
#include "zoomdialog.h"
#include "cutdialog.h"

#define CRAMP 4
#define ERASE 3
#define ZOOM 2
#define FILL 1
#define PEN 0

ToolWidget::ToolWidget(QWidget *parent, QWidget *parent2):r(5),theta(0),phi(0)
{

  QLabel *opacity_label = new QLabel();
  opacity_label->setText("Opacity");

  m_tfwidget = static_cast<TFWidget*>(parent);
  m_renderingwidget = static_cast<RenderingWidget*>(parent2);

  pushButton_Reset = new QPushButton("Reset", this);
  connect(pushButton_Reset, SIGNAL(clicked()), this, SLOT(reset()));

  pushButton_Fill = new QPushButton("Fill", this);
  pushButton_Fill->setCheckable( true );
  pushButton_Fill->setAutoExclusive( true );
  connect(pushButton_Fill, SIGNAL(clicked()), this, SLOT(fill()));

  pushButton_Zoom = new QPushButton("ZoomHistogram", this);
  pushButton_Zoom->setCheckable( true );
  pushButton_Zoom->setAutoExclusive( true );
  connect(pushButton_Zoom, SIGNAL(clicked()), this, SLOT(zoom()));

  pushButton_ZoomArea = new QPushButton("ZoomArea", this);
  pushButton_ZoomArea->setCheckable( true );
  pushButton_ZoomArea->setAutoExclusive( true );
  connect(pushButton_ZoomArea, SIGNAL(clicked()), this, SLOT(zoomarea()));
  /*
  pushButton_Cut = new QPushButton("Set Limit", this);
  pushButton_Cut->setCheckable( true );
  pushButton_Cut->setAutoExclusive( true );
  connect(pushButton_Cut, SIGNAL(clicked()), this, SLOT(cutdialog()));
  */
  pushButton_Color = new QPushButton("Color", this);
  connect(pushButton_Color, SIGNAL(clicked()), this, SLOT(color()));

  //pushButton_Histogram = new QPushButton("Histogram", this);
  //connect(pushButton_Histogram, SIGNAL(clicked()), this, SLOT(histogram()));

  pushButton_Erase = new QPushButton("Erase", this);
  pushButton_Erase->setCheckable( true );
  pushButton_Erase->setAutoExclusive( true );
  connect(pushButton_Erase, SIGNAL(clicked()), this, SLOT(erase()));

  //pushButton_Cramp = new QPushButton("Cramp", this);
  //connect(pushButton_Cramp, SIGNAL(clicked()), this, SLOT(cramp()));

  opacity_slider = new QSlider(Qt::Horizontal, this);
  opacity_slider->setRange(0,255);
  opacity_slider->setSliderPosition(127);
  connect(opacity_slider, SIGNAL(valueChanged(int)), this, SLOT(ChangeOpacitySlider(int)));


  QGridLayout *ToolLayout = new QGridLayout();
  ToolLayout->addWidget( pushButton_Reset, 0, 0 );
  ToolLayout->addWidget( pushButton_Fill, 0, 1 );
  ToolLayout->addWidget( pushButton_Zoom, 0, 3 );
  ToolLayout->addWidget( pushButton_ZoomArea, 0, 4 );
  //ToolLayout->addWidget( pushButton_Cut, 0, 6 );
  ToolLayout->addWidget( pushButton_Color, 1, 0 );
  ToolLayout->addWidget( opacity_slider, 1, 1, 1, 5 );
  ToolLayout->addWidget( pushButton_Erase, 0, 2 );
  ToolLayout->addWidget( opacity_label, 1, 6 );
  //ToolLayout->addWidget( pushButton_Histogram, 0, 4 );
  
  this->setLayout(ToolLayout);
  m_tfwidget->setToolWidget(this);
}


ToolWidget::~ToolWidget()
{
}

void ToolWidget::reset()
{
  std::cout << "Reset" << std::endl;
  m_tfwidget->reset();
  m_renderingwidget->Reset_Rendering();
}

void ToolWidget::fill()
{
  m_tfwidget->setDrawMode(FILL);
}

void ToolWidget::erase()
{
  m_tfwidget->setDrawMode(ERASE);
}

void ToolWidget::cramp()
{
  m_tfwidget->setDrawMode(CRAMP);
}

void ToolWidget::color()
{
  const QColor newColor = QColorDialog::getColor(m_tfwidget->color());
  if (newColor.isValid()){
    m_tfwidget->setPenColor(newColor);
  }
}

void ToolWidget::ChangeOpacitySlider(int opacity)
{
  m_tfwidget->setPenOpacity(opacity);
}

void ToolWidget::apply()
{
  int margin_x = m_tfwidget->return_margin_x();
  int margin_y = m_tfwidget->return_margin_y();
  int tfwindow_x = m_tfwidget->return_tfwindow_x();
  int tfwindow_y = m_tfwidget->return_tfwindow_y();
  int rad = m_tfwidget->PenWidth();

  transferfunction.cmap.resize(256*256);
  transferfunction.omap.resize(256*256);
  for(int i=0;i<256;i++){
    for(int j=0;j<256;j++){
      float opacity = 255.0 - qGreen(m_tfwidget->opacity_image().pixel( margin_x+1+j*rad, 255*rad-i*3 ) );
      transferfunction.setOmap_at( j, i, opacity );
      QColor color;
      color.setRgb(m_tfwidget->color_image().pixel( margin_x+1+j*rad, 255*rad-i*3 ) );
      transferfunction.setCmap_at( j, i, color );
    }
  }
  m_tfwidget->color_image().save(QString("./cmap.png"));
  m_tfwidget->opacity_image().save(QString("./omap.png"));

  m_renderingwidget->apply(transferfunction);
}

void ToolWidget::histogram()
{
}

void ToolWidget::zoom()
{
  m_tfwidget->setDrawMode(ZOOM);
}

void ToolWidget::zoomarea()
{
  ZoomDialog *zoom_dialog;
  zoom_dialog = new ZoomDialog(this);
  zoom_dialog->show();
}

void ToolWidget::cutdialog()
{
  CutDialog *cut_dialog;
  cut_dialog = new CutDialog(this);
  cut_dialog->show();
}

void ToolWidget::public_zoom(float r,float theta ,float phi)
{
  //std::cout << "Clip Area R=" <<r<<" Theta="<<theta<<" Phi="<<phi <<std::endl;
  float radtheta=theta/180.0*M_PI;
  float radphi=phi/180.0*M_PI;
  m_tfwidget->zoomArea(r,radtheta,radphi);
}

void ToolWidget::public_cut(float cut_sei_min, float cut_sei_max, float cut_gt_min, float cut_gt_max)
{
  //std::cout << "Clip Area R=" <<r<<" Theta="<<theta<<" Phi="<<phi <<std::endl;
  m_tfwidget->setLimit(cut_sei_min, cut_sei_max, cut_gt_min, cut_gt_max);
  this->reset();
}

void ToolWidget::setR(float inr)
{
  r=inr;
}
void ToolWidget::setTheta(float intheta)
{
  theta=intheta;
}
void ToolWidget::setPhi(float inphi)
{
  phi=inphi;
}

void ToolWidget::applyrthetaphi()
{
  //std::cout << "Clip Area R=" <<r<<" Theta="<<theta<<" Phi="<<phi <<std::endl;
  float radtheta=theta/180.0*M_PI;
  float radphi=phi/180.0*M_PI;
  m_tfwidget->zoomArea(r,radtheta,radphi);
}
