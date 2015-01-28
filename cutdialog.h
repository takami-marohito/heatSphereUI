#ifndef CUTDIALOG_H
#define CUTDIALOG_H

#include <kvs/qt/Screen>
#include <QList>
#include <QWidget>

#include "toolwidget.h"

 class CutDialog : public QWidget
 {
   Q_OBJECT
   private:
   ToolWidget *m_toolwidget;
   QPushButton *pushButton_Apply;
   QDoubleSpinBox *inputline_sei_min;
   QDoubleSpinBox *inputline_gt_min;
   QDoubleSpinBox *inputline_sei_max;
   QDoubleSpinBox *inputline_gt_max;

float cut_sei_max;
 float cut_gt_max;
float cut_sei_min;
 float cut_gt_min;

   public:
   CutDialog(QWidget *parent);
   ~CutDialog();


   private slots:
   void apply();
   void setsei_min(double inputsei_min);
   void setgt_min(double inputgt_min);
   void setsei_max(double inputsei_max);
   void setgt_max(double inputgt_max);
 };

 #endif
