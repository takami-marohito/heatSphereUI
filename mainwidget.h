#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <kvs/qt/Screen>
#include <QList>
#include <QWidget>
#include <QMainWindow>

#include <string.h>
#include <vector>

#include "rthetaphi.h"
#include "renderingwidget.h"
#include "toolwidget.h"
#include "TFwidget.h"

 class MainWidget : public QWidget
 {
   Q_OBJECT

   private:
   kvs::qt::Screen* m_screen;
   RenderingWidget* m_renderingwidget;
   TFWidget* m_tfwidget;
   ToolWidget* m_toolwidget;
   int m_nsteps;
   int m_nowstep;
   QTimer m_timer;
   RthetaphiSlider *m_slider;
int total_step;

float th_sei;

std::vector<std::string> m_filename;
std::vector<std::string> m_filename_sei;

QAction* left;
QAction* right;
QAction* up;
QAction* down;
QAction* Ctrlleft;
QAction* Ctrlright;
   int value1_step;
int value2_step;


   public:
   MainWidget(kvs::qt::Application* app);
   ~MainWidget();
   void open( std::string filename );
   void opendir( std::vector<std::string> filename, int nsteps, int first_step );
   void opendir( std::vector<std::string> filename, std::vector<std::string> filename_sei, int nsteps, int first_step );
   void open_sei( std::string filename );
   void open_another_time( std::string filename );
   void change_th_sei(float input);
   void open_animation( std::vector<std::string> filename, int nsteps);
   void play_animation(void);
   void stop_animation(void);
   bool animation_rendering_ready;

   private slots:
   void animation_update();
void pressleft();
void pressright();
void pressup();
void pressdown();
void pressCtrlleft();
void pressCtrlright();

 

 };

 #endif

