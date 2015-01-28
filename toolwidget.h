#ifndef TOOLWIDGET_H
#define TOOLWIDGET_H

#include <kvs/qt/Screen>
#include <QList>
#include <QWidget>

#include "renderingwidget.h"
#include "TFwidget.h"
#include "value_structure.h"

class TFWidget;

 class ToolWidget : public QWidget
 {
   Q_OBJECT

   private:
   RenderingWidget *m_renderingwidget;
   TFWidget* m_tfwidget;
   QPushButton *pushButton_Reset;
   QPushButton *pushButton_Fill;
   QPushButton *pushButton_Color;
   QPushButton *pushButton_Cramp;
   QPushButton *pushButton_Erase;
   QPushButton *pushButton_Histogram;
   QPushButton *pushButton_Zoom;
   QPushButton *pushButton_ZoomArea;
   QPushButton *pushButton_Cut;

   QSlider* opacity_slider;

   float r;
   float theta;
   float phi;

   private slots:
   void reset();
   void fill();
   void cramp();
   void color();
   void erase();
   void ChangeOpacitySlider(int opacity);
   void histogram();
   void zoom();
   void zoomarea();
   void cutdialog();

   public:
   ToolWidget(QWidget *parent, QWidget *parent2);
   ~ToolWidget();
   TransferFunction2D transferfunction;
   void public_zoom(float r,float theta,float phi);
   void public_cut(float cut_sei_min, float cut_sei_max, float cut_gt_min, float cut_gt_max);
   void apply();
   void setR(float inr);
   void setTheta(float intheta);
   void setPhi(float inphi);
   void applyrthetaphi();
 };

 #endif

