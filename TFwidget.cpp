#include "TFwidget.h"

#include <kvs/qt/Screen>
#include <QListWidget>
#include <string.h>

#include <QWidget>
#include <QPainter>
#include <algorithm>
#include "value_structure.h"

#include "renderingwidget.h"

#include <math.h>

#define ERASE 3
#define ZOOM 2
#define FILL 1
#define PEN 0

namespace
{
  int TFWindowX = 868;
  int TFWindowY = 838;
  int margin_x = 100;
  int margin_y = 70;
  int rad = 3;  //dot per one transferfunction
  bool load_flag = false;
}

TFWidget::TFWidget(kvs::qt::Application* app)
  :draw_mode(0),m_color(QColor(255,0,0)),m_opacity(127)
{
  this->setFixedSize(TFWindowX, TFWindowY);
  //this->setMinimumSize(margin_x+100, margin_y+100);
  m_draw_image = QImage(TFWindowX, TFWindowY, QImage::Format_ARGB32);
  m_color_image = QImage(TFWindowX, TFWindowY, QImage::Format_ARGB32);
  m_opacity_image = QImage(TFWindowX, TFWindowY, QImage::Format_ARGB32);
  QPainter painter(&m_draw_image);
  painter.fillRect(0, 0, TFWindowX, TFWindowY, QColor(255,255,255) );
  m_color_image = m_draw_image;
  m_opacity_image = m_draw_image;
  painter.end();
  //this->setFixedSize(TFWindowX, TFWindowY);
  //this->resize(868, 838);
  this->show();
}

void TFWidget::paintEvent( QPaintEvent *event )
{
  QPainter painter(this);
  QRect dirtyRect = event->rect();
  painter.drawImage(dirtyRect, m_draw_image, dirtyRect);
}


TFWidget::~TFWidget()
{
}

void TFWidget::setHistogram(std::vector<int> &histogram)
{
  m_histogram.resize(histogram.size());
  std::copy(histogram.begin(), histogram.end(), m_histogram.begin());
}

void TFWidget::drawHistogram()
{
  drawAxis();
  m_org_histogram_image = QImage(256, 256, QImage::Format_ARGB32);
  QPainter painter(&m_org_histogram_image);
  
  std::vector<bool> histogram_flag(256*256, false);
  m_tfunc2d.clear();
  m_tfunc2d.resize(256*256,0);

  QPen tmp_pen;

  for(int i=0;i<256;i++){
    tmp_pen.setWidth( 1 );
    tmp_pen.setColor(QColor(255-i,255-i,255-i));
    painter.setPen( tmp_pen );
     QPolygonF samplepoint;
     for(int j=0;j<256;j++){//j is image y
       for(int k=0;k<256;k++){//k is image x
	 if(histogram_flag.at(j*256+k) == false){
	   if(i == m_histogram.at(j*256+k) ){
	     samplepoint += QPointF(k,255-j);
	     histogram_flag.at(j*256+k) = true;
	     m_tfunc2d.omap.at(j*256+k)= 255-i;
	     //m_tfunc2d.cmap.at(j*256+k).setRed(i);
	     //m_tfunc2d.cmap.at(j*256+k).setGreen(i);
	     //m_tfunc2d.cmap.at(j*256+k).setBlue(i);
	   }
	 }
       }
     }
     painter.drawPoints(samplepoint);     
  }
  QPainter painter2(&m_draw_image);
  QImage newImage = m_org_histogram_image.scaled(QSize(TFWindowX-margin_x,TFWindowY-margin_y), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  //SmoothTransformation uses bilinear filtering
  painter2.drawImage(margin_x,0,newImage);
  update();
  m_histogram_image = m_draw_image;
  printf("drawHistogram\n");
}


void TFWidget::drawAxis()
{
  QPainter painter(&m_draw_image);
  //std::cout << "drawAxis" << std::endl;

  this->setHistogramMinMax(m_renderingwidget->tmp_min_1,m_renderingwidget->tmp_max_1, m_renderingwidget->tmp_min_2, m_renderingwidget->tmp_max_2);

  char max_c1[20];
  char min_c1[20];
  char max_c2[20];
  char min_c2[20];
  char avg_c1[20];
  char avg_c2[20];
  char memori_2_c1[20];
  char memori_3_c1[20];
  char memori_2_c2[20];
  char memori_3_c2[20];
  //printf("%f %f %f %f\n",data2_max, data2_min, data1_max, data1_min);
  
  sprintf(max_c1, "%7.2f",data1_max);
  sprintf(min_c1, "%7.2f",data1_min);
  sprintf(max_c2, "%7.2f",data2_max);
  sprintf(min_c2, "%7.2f",data2_min);
  sprintf(avg_c1, "%7.2f",(data1_max+data1_min)/2);
  sprintf(avg_c2, "%7.2f",(data2_max+data2_min)/2);
  sprintf(memori_2_c1, "%7.2f",(data1_max-data1_min)/4+data1_min);
  sprintf(memori_2_c2, "%7.2f",(data2_max-data2_min)/4+data2_min);
  sprintf(memori_3_c1, "%7.2f",(data1_max-data1_min)*3/4+data1_min);
  sprintf(memori_3_c2, "%7.2f",(data2_max-data2_min)*3/4+data2_min);
  /*
  sprintf(max_c1, "%f",data1_max);
  sprintf(min_c1, "%f",data1_min);
  sprintf(avg_c1, "%f",pow(10.0,(data1_max+data1_min)/2)-1);
  sprintf(memori_2_c1, "%f",(data1_max+data1_min)/4);
  sprintf(memori_3_c1, "%f",(data1_max+data1_min)*3/4);
  sprintf(max_c2, "%f",pow(10.0,data2_max));
  sprintf(min_c2, "%f",pow(10.0,data2_min));
  sprintf(avg_c2, "%f",pow(10.0,(data2_max+data2_min)/2));
  sprintf(memori_2_c2, "%f",pow(10.0,(data2_max-data2_min)/4+data2_min));
  sprintf(memori_3_c2, "%f",pow(10.0,(data2_max-data2_min)*3/4+data2_min));
  */
  /*
  sprintf(max_c1, "%f",data1_max);
  sprintf(min_c1, "%f",data1_min);
  sprintf(max_c2, "%f",pow(10.0,pow(10.0,data2_max)));
  sprintf(min_c2, "%f",pow(10.0,pow(10.0,data2_min)));
  sprintf(avg_c1, "%f",(data1_max+data1_min)/2);
  sprintf(avg_c2, "%f",pow(10.0,pow(10.0,(data2_max+data2_min)/2)));
  sprintf(memori_2_c1, "%f",(data1_max+data1_min)/4);
  sprintf(memori_2_c2, "%f",pow(10.0,pow(10.0,(data2_max-data2_min)/4+data2_min)));
  sprintf(memori_3_c1, "%f",(data1_max+data1_min)*3/4);
  sprintf(memori_3_c2, "%f",pow(10.0,pow(10.0,(data2_max-data2_min)*3/4+data2_min)));
  */
  setHistogramMinMax(atof(min_c1), atof(max_c1), atof(min_c2), atof(max_c2));

  painter.eraseRect(0,0,TFWindowX,TFWindowY);
  painter.setPen( QPen(QColor(0,0,0 ) ,3.0) );
  painter.drawLine(QPoint(margin_x-20,TFWindowY-margin_y+1),QPoint(TFWindowX,TFWindowY-margin_y+1) ); //軸
  painter.drawLine(QPoint(margin_x-2,0),QPoint(margin_x-2,TFWindowY-margin_y+20));                    //軸
  painter.drawText(QRect(1,300*TFWindowY/(this->height()-margin_y),margin_x,margin_x), Qt::AlignLeft, tr(m_data2.name.c_str() ));         //y軸の名前
  painter.drawText(QRect(TFWindowX-margin_x-100,TFWindowY-20,150,margin_y), Qt::AlignLeft, tr(m_data1.name.c_str() ));

  painter.drawLine(QPoint(margin_x-2,1),QPoint(margin_x-20,1)); //y軸の上限目盛り
  painter.drawLine(QPoint(margin_x-2,(TFWindowY-margin_y)/2),QPoint(margin_x-20,(TFWindowY-margin_y)/2)); //y軸の真ん中目盛り
  painter.drawLine(QPoint((TFWindowX+margin_x-2)/2,(TFWindowY-margin_y+1)),QPoint((TFWindowX+margin_x-2)/2,(TFWindowY-margin_y)+20 )); //x軸の真ん中目盛り
  painter.drawLine(QPoint(TFWindowX-2,(TFWindowY-margin_y)+1),QPoint(TFWindowX-2,(TFWindowY-margin_y)+20 )); //x軸の上限目盛り

  painter.drawLine(QPoint(margin_x-2,(TFWindowY-margin_y)*3/4),QPoint(margin_x-20,(TFWindowY-margin_y)*3/4)); //y軸の0.25目盛り
  painter.drawLine(QPoint((TFWindowX-margin_x-2)*3/4+margin_x,(TFWindowY-margin_y+1)),QPoint((TFWindowX-margin_x-2)*3/4+margin_x,(TFWindowY-margin_y)+20 )); //x軸の0.75目盛り
  painter.drawLine(QPoint(margin_x-2,(TFWindowY-margin_y)/4),QPoint(margin_x-20,(TFWindowY-margin_y)/4)); //y軸の0.75目盛り
  painter.drawLine(QPoint((TFWindowX-margin_x-2)/4+margin_x,(TFWindowY-margin_y+1)),QPoint((TFWindowX-margin_x-2)/4+margin_x,(TFWindowY-margin_y)+20 )); //x軸の0.25目盛り

  painter.drawText(QRect(5,5,margin_x,margin_x), Qt::AlignLeft, tr(max_c2 ));//y軸上限
  painter.drawText(QRect(TFWindowX-100,TFWindowY-40,100,40), Qt::AlignRight, tr(max_c1 ));//x軸上限
  painter.drawText(QRect(5,(TFWindowY-margin_y)/2,margin_x,margin_x), Qt::AlignLeft, tr(avg_c2 ));//y軸真ん中
  painter.drawText(QRect((TFWindowX-margin_x)/2+30,TFWindowY-40,100,40), Qt::AlignRight, tr(avg_c1 ));//x軸真ん中
  painter.drawText(QRect(5,(TFWindowY-margin_y),margin_x,margin_y), Qt::AlignLeft, tr(min_c2 ));//y下限
  painter.drawText(QRect(margin_x,TFWindowY-40,100,40), Qt::AlignLeft, tr(min_c1 ));//x下限
  painter.drawText(QRect((TFWindowX-margin_x)*3/4+margin_x-30,(TFWindowY-40),margin_x,margin_y), Qt::AlignLeft, tr(memori_3_c1 ));//x0.75
  painter.drawText(QRect((TFWindowX-margin_x)/4+margin_x-30,(TFWindowY-40),margin_x,margin_y), Qt::AlignLeft, tr(memori_2_c1 ));//x0.25
  painter.drawText(QRect(5,(TFWindowY-margin_y)/4,margin_x,margin_x), Qt::AlignLeft, tr(memori_3_c2 ));//y0.75
  painter.drawText(QRect(5,(TFWindowY-margin_y)*3/4,margin_x,margin_x), Qt::AlignLeft, tr(memori_2_c2 ));//y0.25
  painter.end();
}

void TFWidget::setValueStruct(ValueStruct input1, ValueStruct input2)
{
  m_data1.setData(input1.value, input1.name);
  m_data2.setData(input2.value, input2.name);
  //data1_max = *std::max_element( m_data1.value.begin(), m_data1.value.end() );
  //data1_min = *std::min_element( m_data1.value.begin(), m_data1.value.end() );
  //data2_max = *std::max_element( m_data2.value.begin(), m_data2.value.end() );
  //data2_min = *std::min_element( m_data2.value.begin(), m_data2.value.end() );
}

void TFWidget::setPenColor(QColor inputcolor)
{
  m_color = inputcolor;
}
 
void TFWidget::setPenOpacity(int opacity)
{
  m_opacity = opacity;
}

void TFWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    m_mouse_position = event->pos();
  }
  if(draw_mode == FILL || draw_mode == ZOOM || draw_mode == ERASE){
    m_fill_startpoint = event->pos();
    m_saved_fill_draw_image = m_draw_image;
  }
}

void TFWidget::mouseMoveEvent(QMouseEvent *event)
{
  if(draw_mode == FILL || draw_mode == ERASE || draw_mode == ZOOM){
    QPoint point_end = event->pos();
    QPoint point3(m_fill_startpoint.x(), point_end.y());
    QPoint point4(point_end.x(), m_fill_startpoint.y());
    QLine line1(m_fill_startpoint, point3);
    QLine line2(m_fill_startpoint, point4);
    QLine line3(point_end, point3);
    QLine line4(point_end, point4);

    QPainter painter(&m_draw_image);
    painter.drawImage(0,0,m_saved_fill_draw_image);
    painter.setPen( QPen(Qt::black, 2, Qt::DashLine) );
    painter.drawLine(line1);
    painter.drawLine(line2);
    painter.drawLine(line3);
    painter.drawLine(line4);
    update();
  }else{
    if ((event->buttons() & Qt::LeftButton)){
      drawLineTo(event->pos());
    }
  }
}

void TFWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if(draw_mode == FILL){
    fillSquare(m_fill_startpoint, event->pos());
    if(m_fill_startpoint != event->pos()){
      m_toolwidget->apply();
    }
  }else if(draw_mode == ZOOM){
    if(m_fill_startpoint != event->pos()){
      zoomSquare(m_fill_startpoint, event->pos());
    }
  }else if(draw_mode == ERASE){
    eraseSquare(m_fill_startpoint, event->pos());
    if(m_fill_startpoint != event->pos()){
      m_toolwidget->apply();
    }
  }else{
    if (event->button() == Qt::LeftButton) {
      drawLineTo(event->pos());
    }
  }


}

void TFWidget::drawLineTo(const QPoint &drawpoint)
{
}

void TFWidget::fillSquare(QPoint start, QPoint end)
{
  int vertical = start.y()-end.y();
  int horizontal = start.x()-end.x();
  int top = end.y();
  int left = end.x();
  if(vertical < 0){
    vertical = (-1)*vertical;
    top = start.y();
  }
  if(horizontal<0){
    horizontal = (-1)*horizontal;
    left = start.x();
  }

 QPainter painter(&m_draw_image);
 painter.drawImage(0,0,m_saved_fill_draw_image);
 QRegion region(QRect(margin_x, 0, TFWindowX, TFWindowY-margin_y));
 painter.setClipRegion(region);
 QRect tar(left,top,horizontal,vertical);
 QRect src(left,top,horizontal,vertical);
 painter.drawImage(tar, m_histogram_image, src);
 painter.setOpacity( (float)m_opacity/255.0 );
 painter.fillRect(left, top, horizontal, vertical, m_color );

 QPainter painter_color(&m_color_image);
 painter_color.setClipRegion(region);
 painter_color.fillRect(left, top, horizontal, vertical, m_color );

 QPainter painter_opacity(&m_opacity_image);
 QColor opacity_color(0, 0, 0);
 //QColor opacity_color(0, 0, m_opacity);
 painter_opacity.setClipRegion(region);
 painter_opacity.fillRect(left, top, horizontal, vertical, QColor(255,255,255) );
 painter_opacity.setOpacity( (float)m_opacity/255.0 );
 painter_opacity.fillRect(left, top, horizontal, vertical, opacity_color );
 update();
}

void TFWidget::zoomSquare(QPoint start, QPoint end)
{
  int vertical = start.y()-end.y();
  int horizontal = start.x()-end.x();
  int top = end.y();
  int left = end.x();
  if(vertical < 0){
    vertical = (-1)*vertical;
    top = start.y();
  }
  if(horizontal<0){
    horizontal = (-1)*horizontal;
    left = start.x();
  }

  f_zoom_startx = (float)(left-margin_x+1)/(float)(TFWindowX-margin_x);
  f_zoom_starty = (float)(top)/(float)(TFWindowY-margin_y);
  
  f_zoom_vertical = (float)(vertical)/(float)(TFWindowY-margin_y);
  f_zoom_horizontal = (float)(horizontal)/(float)(TFWindowX-margin_x);

  m_renderingwidget->CreateHistogramZooming(m_data1.value, m_data2.value, f_zoom_startx, f_zoom_starty, f_zoom_vertical, f_zoom_horizontal);

  this->setHistogram(m_renderingwidget->histogram_zoom);
  this->setHistogramMinMax(m_renderingwidget->tmp_min_1,m_renderingwidget->tmp_max_1, m_renderingwidget->tmp_min_2, m_renderingwidget->tmp_max_2);
  this->drawHistogram();
}

void TFWidget::eraseSquare(QPoint start, QPoint end)
{
  int vertical = start.y()-end.y();
  int horizontal = start.x()-end.x();
  int top = end.y();
  int left = end.x();
  if(vertical < 0){
    vertical = (-1)*vertical;
    top = start.y();
  }
  if(horizontal<0){
    horizontal = (-1)*horizontal;
    left = start.x();
  }
 QPainter painter(&m_draw_image);
 painter.drawImage(0,0,m_saved_fill_draw_image);
 QRegion region(QRect(margin_x, 0, TFWindowX, TFWindowY-margin_y));
 painter.setClipRegion(region);
 QRect tar(left,top,horizontal,vertical);
 QRect src(left,top,horizontal,vertical);
 painter.drawImage(tar, m_histogram_image, src);

 QPainter painter_color(&m_color_image);
 painter_color.setClipRegion(region);
 painter_color.fillRect(left, top, horizontal, vertical, QColor(255,255,255) );

 QPainter painter_opacity(&m_opacity_image);
 painter_opacity.setClipRegion(region);
 painter_opacity.fillRect(left, top, horizontal, vertical, QColor(255,255,255) );
 update();
}

void TFWidget::setDrawMode(int input_mode)
{
  draw_mode = input_mode;
}

int TFWidget::PenWidth()
{
  return(rad);
}

int TFWidget::return_margin_x()
{
  return(margin_x);
}

int TFWidget::return_margin_y()
{
  return(margin_y);
}

int TFWidget::return_tfwindow_x()
{
  return(margin_x);
}

int TFWidget::return_tfwindow_y()
{
  return(margin_y);
}

void TFWidget::reset()
{
  m_renderingwidget->CreateHistogram(m_data1.value,m_data2.value,m_renderingwidget->surface_node);
  this->setHistogram(m_renderingwidget->histogram);
  this->setHistogramMinMax(m_renderingwidget->tmp_min_1,m_renderingwidget->tmp_max_1, m_renderingwidget->tmp_min_2, m_renderingwidget->tmp_max_2);

  drawHistogram();
  QImage tmp_image = QImage(TFWindowX, TFWindowY, QImage::Format_ARGB32);
  QPainter painter(&tmp_image);
  painter.fillRect(0, 0, TFWindowX, TFWindowY, QColor(255,255,255) );
  m_color_image = tmp_image;
  m_opacity_image = tmp_image;
}

void TFWidget::RenderingNthStep(int step)
{

}

void TFWidget::open_animation(std::vector<std::string> filename, int nsteps)
{

}


void TFWidget::setRenderingWidget(RenderingWidget* input_renderingwidget)
{
  m_renderingwidget = input_renderingwidget;
}

void TFWidget::setToolWidget(ToolWidget* input_toolwidget)
{
  m_toolwidget = input_toolwidget;
}

void TFWidget::setHistogramMinMax(float min1, float max1, float min2, float max2)
{
  //data1_max = *std::max_element( m_data1.value.begin(), m_data1.value.end() );
  //data1_min = *std::min_element( m_data1.value.begin(), m_data1.value.end() );
  //data2_max = *std::max_element( m_data2.value.begin(), m_data2.value.end() );
  //data2_min = *std::min_element( m_data2.value.begin(), m_data2.value.end() );
  data1_max = max1;
  data1_min = min1;
  data2_max=max2;
  data2_min = min2;
}

void TFWidget::resizeEvent(QResizeEvent *event)
{
  TFWindowX = this->width();
  TFWindowY = this->height();

  //m_draw_image = QImage(TFWindowX, TFWindowY, QImage::Format_ARGB32);
  m_color_image = QImage(TFWindowX, TFWindowY, QImage::Format_ARGB32);
  m_opacity_image = QImage(TFWindowX, TFWindowY, QImage::Format_ARGB32);
  //QPainter painter(&m_draw_image);
  //painter.fillRect(0, 0, TFWindowX, TFWindowY, QColor(255,255,255) );
  //m_color_image = m_draw_image;
  //m_opacity_image = m_draw_image;
  //drawHistogram();
  
}


void TFWidget::zoomArea(float r,float theta,float phi)
{
  m_renderingwidget->CreateHistogramZooming(m_data1.value, m_data2.value, r,theta,phi);

  this->setHistogram(m_renderingwidget->histogram_zoom);
  this->drawHistogram();
}

void TFWidget::setLimit(float cut_sei_min, float cut_sei_max, float cut_gt_min, float cut_gt_max)
{
  if(cut_sei_min > cut_sei_max || cut_gt_min>cut_gt_max){
    std::cout<<"it cannot be set max GradT < min GradT" << std::endl;
    return;
  }
  std::cout<<"set Limit" << std::endl;
  m_renderingwidget->setLimit(cut_sei_min, cut_sei_max, cut_gt_min, cut_gt_max);

  this->reset();
}
