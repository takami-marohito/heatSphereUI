#ifndef TFWIDGET_H
#define TFWIDGET_H

#include <kvs/qt/Screen>
#include <QList>
#include <QWidget>

#include <QImage>
#include <vector>
#include <string.h>

#include "renderingwidget.h"
#include "value_structure.h"
#include "toolwidget.h"

class ToolWidget;

 class TFWidget : public QWidget
 {
   Q_OBJECT

   private:
   QColor m_color;
   int m_opacity;
   QImage m_draw_image;
   QImage m_histogram_image;
   QImage m_org_histogram_image;
   QImage m_color_image;
   QImage m_opacity_image;
   QImage m_saved_fill_draw_image;
   QImage m_tmp_draw_image;
   std::vector<int> m_histogram;
   void drawAxis();
   std::string m_x_name;
   std::string m_y_name;
   ValueStruct m_data1, m_data2;
   TransferFunction2D m_tfunc2d;
   QPoint m_mouse_position;
   QPoint m_fill_startpoint;
   int draw_mode;
   void drawLineTo(const QPoint &drawpoint);
   void fillSquare(QPoint start, QPoint end);
   void eraseSquare(QPoint start, QPoint end);
   void zoomSquare(QPoint start, QPoint end);
   RenderingWidget* m_renderingwidget;
   ToolWidget* m_toolwidget;

   protected:
   void paintEvent( QPaintEvent *event );
   void mousePressEvent(QMouseEvent *event);
   void mouseMoveEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *event);
   void resizeEvent(QResizeEvent *event);

   public:
   TFWidget(kvs::qt::Application* app);
   ~TFWidget();
   void setHistogram(std::vector<int> &histogram);
   void drawHistogram();
   void setValueStruct(ValueStruct input1, ValueStruct input2);
   void setPenColor(QColor inputcolor);
   void setPenOpacity(int opacity);
   void setDrawMode(int input_mode);
   QColor color(){return(m_color);};
   QImage opacity_image(){return(m_opacity_image);};
   QImage color_image(){return(m_color_image);};
   int PenWidth();
   int return_margin_y();
   int return_margin_x();
   int return_tfwindow_x();
   int return_tfwindow_y();

   void setRenderingWidget(RenderingWidget* input_renderingwidget);
   void setToolWidget(ToolWidget* input_toolwidget);
   void reset();
   void RenderingNthStep(int step);
   void open_animation(std::vector<std::string> filename, int nsteps);

   std::vector<int> Histogram(){return(m_histogram);};

   float f_zoom_startx;
   float f_zoom_starty;
   float f_zoom_vertical;
   float f_zoom_horizontal;

   float data1_min;
   float data1_max;
   float data2_min;
   float data2_max;

   void setHistogramMinMax(float min1, float max1, float min2, float max2);

   void zoomArea(float r,float theta,float phi);

   void setLimit(float cut_sei_min, float cut_sei_max, float cut_gt_min, float cut_gt_max);

   RenderingWidget* returnRenderingWidget(){return(m_renderingwidget);};
 };

 #endif

