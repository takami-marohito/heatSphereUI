#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kvs/qt/Screen>
#include <QMainWindow>

#include "mainwidget.h"

 class MainWindow : public QMainWindow
 {
   Q_OBJECT
     bool animation_flag;

     private:
   MainWidget* m_mainwidget;
   kvs::qt::Screen* m_screen;
   QMenu* m_file_menu;
   QMenu* m_GradT_menu;
   QMenu* m_SecondInvariant_menu;
   QAction* m_open_action;
   QAction* m_exit_action;
   QAction* m_open_sei_action;
   QAction* m_open_animation_action;
   QAction* m_open_another_time_action;
   QAction* m_change_th_sei_action;
   bool is_animation;

   public:
   MainWindow(kvs::qt::Application* app);
   ~MainWindow();

   private slots:
   void open();
   void exit();
   void open_sei();
   void change_th_sei();
   void open_another_time();
   void open_animation();
  protected:
virtual void keyPressEvent(QKeyEvent* event);
 };

 #endif
