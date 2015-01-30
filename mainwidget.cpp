#include "mainwidget.h"

#include <kvs/qt/Screen>
#include <QListWidget>
#include <string.h>
#include <QTimer>
#include <vector>

MainWidget::MainWidget(kvs::qt::Application* app):m_nsteps(0),m_nowstep(0),animation_rendering_ready(true)
{
  value1_step = -1;
  value2_step = -1;
  total_step = -1;

  m_renderingwidget = new RenderingWidget( app );
  m_tfwidget = new TFWidget( app );
  m_toolwidget = new ToolWidget( m_tfwidget, m_renderingwidget );
  m_slider = new RthetaphiSlider( m_toolwidget );

  QGridLayout *mainLayout = new QGridLayout();
  mainLayout->addWidget( m_renderingwidget, 0, 0, 8, 5 );
  mainLayout->addWidget( m_tfwidget, 0, 5, 8, 5 );
  mainLayout->addWidget( m_toolwidget, 8, 5, 2, 5 );
  mainLayout->addWidget( m_slider, 9, 0 );
  this->setLayout(mainLayout);

  left  = new QAction(this);
  right  = new QAction(this);
  up  = new QAction(this);
  down  = new QAction(this);
  Ctrlleft  = new QAction(this);
  Ctrlright  = new QAction(this);

  this->addAction(left);
  this->addAction(right);
  this->addAction(up);
  this->addAction(down);
  this->addAction(Ctrlleft);
  this->addAction(Ctrlright);

  connect(left, SIGNAL(triggered()), this, SLOT(pressleft()));
  connect(right, SIGNAL(triggered()), this, SLOT(pressright()));
  connect(up, SIGNAL(triggered()), this, SLOT(pressup()));
  connect(down, SIGNAL(triggered()), this, SLOT(pressdown()));
  connect(Ctrlleft, SIGNAL(triggered()), this, SLOT(pressCtrlleft()));
  connect(Ctrlright, SIGNAL(triggered()), this, SLOT(pressCtrlright()));

  left->setShortcut(Qt::Key_Left);
  right->setShortcut(Qt::Key_Right);
  up->setShortcut(Qt::Key_Up);
  down->setShortcut(Qt::Key_Down);
  Ctrlleft->setShortcut(Qt::CTRL + Qt::Key_Left);
  Ctrlright->setShortcut(Qt::CTRL + Qt::Key_Right);

  connect(&m_timer, SIGNAL(timeout()), this, SLOT(animation_update()) );
}

MainWidget::~MainWidget()
{
}

void MainWidget::open( std::string filename )
{
  m_renderingwidget->open(filename);
  m_tfwidget->setHistogram(m_renderingwidget->histogram);
  m_tfwidget->setValueStruct(m_renderingwidget->histogramdata1, m_renderingwidget->histogramdata2);
  m_tfwidget->setRenderingWidget(m_renderingwidget);
  m_tfwidget->drawHistogram();
}

void MainWidget::opendir( std::vector<std::string> filename, int nsteps, int first_step )
{
  for(unsigned int i=0;i<filename.size();i++){
    m_filename.push_back(filename.at(i));
  }
  value1_step = first_step;
  value2_step = first_step;
  total_step = nsteps;
  th_sei = 5;

  printf("start loading %s\n",filename.at(first_step).c_str());
  m_renderingwidget->open(m_filename.at(first_step));
  m_tfwidget->setHistogram(m_renderingwidget->histogram);
  m_tfwidget->setValueStruct(m_renderingwidget->histogramdata1, m_renderingwidget->histogramdata2);
  m_tfwidget->setRenderingWidget(m_renderingwidget);
  m_tfwidget->drawHistogram();
}

void MainWidget::opendir( std::vector<std::string> filename, std::vector<std::string> filename_sei, int nsteps, int first_step )
{
  for(unsigned int i=0;i<filename.size();i++){
    m_filename.push_back(filename.at(i));
  }
  for(unsigned int i=0;i<filename_sei.size();i++){
    m_filename_sei.push_back(filename_sei.at(i));
  }
  value1_step = first_step;
  value2_step = first_step;
  total_step = nsteps;
  th_sei = 5;

  printf("start loading %s\n",filename.at(first_step).c_str());
  m_renderingwidget->open(m_filename.at(first_step), m_filename_sei.at(first_step));
  m_tfwidget->setHistogram(m_renderingwidget->histogram);
  m_tfwidget->setValueStruct(m_renderingwidget->histogramdata1, m_renderingwidget->histogramdata2);
  m_tfwidget->setRenderingWidget(m_renderingwidget);
  m_tfwidget->drawHistogram();
}

void MainWidget::open_sei( std::string filename )
{
  m_renderingwidget->open_sei(filename);
  m_tfwidget->setHistogram(m_renderingwidget->histogram);
  m_tfwidget->setValueStruct(m_renderingwidget->datastruct1, m_renderingwidget->datastruct2);
  m_tfwidget->drawHistogram();
}

void MainWidget::open_animation( std::vector<std::string> filename, int nsteps)
{
  m_nsteps = nsteps;
  m_renderingwidget->open_animation(filename, nsteps);
  //m_tfwidget->open_animation(filename, nsteps);
  m_tfwidget->setHistogram(m_renderingwidget->histogram);
  m_tfwidget->setValueStruct(m_renderingwidget->datastruct1, m_renderingwidget->datastruct2);
  m_tfwidget->drawHistogram();
}

void MainWidget::play_animation()
{
  if(m_nsteps > 1){
    m_timer.setInterval(500);
    m_timer.start();
  }else{
    printf("must load animation data at first. (at least 2 files)\n");
  }
}

void MainWidget::stop_animation()
{
  m_timer.stop();
}

void MainWidget::animation_update()
{
  if(animation_rendering_ready == true){
    animation_rendering_ready = false;
    m_nowstep++;
    if(m_nowstep == m_nsteps){
      stop_animation();
      m_nowstep = 0;
      m_renderingwidget->RenderingNthStep(m_nowstep);
      m_tfwidget->RenderingNthStep(m_nowstep);
    }else{
      m_renderingwidget->RenderingNthStep(m_nowstep);
      m_tfwidget->RenderingNthStep(m_nowstep);
      //m_timer.setInterval(500);
      //m_timer.start();
    }
    animation_rendering_ready = true;
  }
}

void MainWidget::open_another_time( std::string filename )
{
  m_renderingwidget->open_another_time(filename);
  m_tfwidget->setHistogram(m_renderingwidget->histogram);
  m_tfwidget->setValueStruct(m_renderingwidget->datastruct1, m_renderingwidget->datastruct2);
  m_tfwidget->drawHistogram();
}

void MainWidget::change_th_sei( float input )
{
  m_renderingwidget->change_th_sei(input);
  m_tfwidget->setHistogram(m_renderingwidget->histogram_zoom);
  m_tfwidget->setValueStruct(m_renderingwidget->datastruct1, m_renderingwidget->datastruct2);
  m_tfwidget->drawHistogram();
}

void MainWidget::pressleft()
{
  if(value2_step>-1){
    if(value2_step == 0){
      printf("value2 step is minimum\n");
    }else{
      value2_step--;
      this->open_another_time(m_filename.at(value2_step));
      printf("loading %s done\n",m_filename.at(value2_step).c_str());
    }
  }
}

void MainWidget::pressright()
{
  if(value2_step>-1){
    if(value2_step == total_step-1){
      printf("value2 step is maximum\n");
    }else{
      value2_step++;
      this->open_another_time(m_filename.at(value2_step));
      printf("loading %s done\n",m_filename.at(value2_step).c_str());
    }
  }
}
void MainWidget::pressup()
{
  th_sei = th_sei/0.7;
  this->change_th_sei(th_sei);
}
void MainWidget::pressdown()
{
  th_sei = th_sei*0.7;
  if(th_sei < 0){
    printf("the threshold of second invariant is minimum\n");
    th_sei = th_sei/0.7;
  }else{
    this->change_th_sei(th_sei);
  }
}

void MainWidget::pressCtrlleft()
{
}
void MainWidget::pressCtrlright()
{
}
