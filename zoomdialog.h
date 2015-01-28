#ifndef ZOOMDIALOG_H
#define ZOOMDIALOG_H

#include <kvs/qt/Screen>
#include <QList>
#include <QWidget>

#include "toolwidget.h"

 class ZoomDialog : public QWidget
 {
   Q_OBJECT
   private:
   ToolWidget *m_toolwidget;
   QPushButton *pushButton_Apply;
   QDoubleSpinBox *inputline_r;
   QDoubleSpinBox *inputline_theta;
   QDoubleSpinBox *inputline_phi;

float r;
float theta;
float phi;

   public:
   ZoomDialog(QWidget *parent);
   ~ZoomDialog();

   void setRange(float input_min1, float input_max1, float input_min2, float input_max2);

   private slots:
   void apply();
   void setR(double inputr);
   void setTheta(double inputtheta);
   void setPhi(double inputphi);
 };

 #endif
