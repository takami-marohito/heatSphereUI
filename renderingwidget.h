#ifndef RENDERINGWIDGET_H
#define RENDERINGWIDGET_H

#include <kvs/qt/Screen>
#include <QList>
#include <QWidget>
#include <QMainWindow>
#include <kvs/TransferFunction>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/StochasticRenderingCompositor>
#include <kvs/glut/Label>

#include <QSlider>

#include <string.h>

#include "value_structure.h"

#include <kvs/PolygonObject>

namespace
{
  int m_step=0;
}

 class RenderingWidget : public QWidget
 {
   Q_OBJECT

   private:
   kvs::UnstructuredVolumeObject *object1;
   std::vector<unsigned int> surface_polygon_connection;

unsigned int SecondInvariantColor(float secondinvariant);
unsigned int GradTColor(float gradt);
float CutSecondInvariant(float secondinvariant);
float CutGradT(float gradt);
float HistogramCutSecondInvariant(float secondinvariant);
float HistogramCutGradT(float gradt);

   double CalcTimesLength(std::vector<float> vec1, std::vector<float> vec2, std::vector<unsigned int> surface_node_area);

   kvs::PolygonObject* pobject1;
   kvs::PolygonObject* pobject2;
   kvs::PolygonObject* pobject_background;
   kvs::StochasticRenderingCompositor *compositor;
   kvs::qt::Screen* m_screen;
   float* m_tfunc2d;
std::string m_filename;
std::vector<float> m_SecondInvariant;

   std::vector<int> histogramForUpdate;
   float LengthAandB(float ax, float ay, float az, float bx, float by, float bz);


   public:
   float cut_value_sei_min;
   float cut_value_gt_min;
   float cut_value_sei_max;
   float cut_value_gt_max;
   float histogram_cut_value_sei_min;
   float histogram_cut_value_gt_min;
   float histogram_cut_value_sei_max;
   float histogram_cut_value_gt_max;
   RenderingWidget(kvs::qt::Application* app);
   ~RenderingWidget();
   void open( std::string filename );
void open( std::string filename, std::string filename_sei);
   void open_sei( std::string filename );
  void open_sei( std::string filename, std::string filename_sei );
   void open_another_time( std::string filename );
   void change_th_sei(float input);
   void open_animation( std::vector<std::string> filename, int nsteps );
   void CreateHistogram(std::vector<float> &value1, std::vector<float> &value2, std::vector<unsigned int> &index_matrix);

   void CreateHistogramZooming(std::vector<float> &value1, std::vector<float> &value2, float f_startx, float f_starty, float f_vertical, float f_horizontal);

  void CreateHistogramZooming(std::vector<float> &value1, std::vector<float> &value2, float inr, float intheta, float inphi);

   void LoadHistogram(std::string filename);
   std::vector<int> histogram;
   std::vector<int> histogram_zoom;
   ValueStruct datastruct1;
   ValueStruct datastruct2;
ValueStruct histogramdata1;
ValueStruct histogramdata2;

   void apply( TransferFunction2D input_tfunc );
   void RenderingNthStep(int step);

   float r;
   float theta;
   float phi;
   bool spot_flag;

   float tmp_min_1;
   float tmp_max_1;
   float tmp_min_2;
   float tmp_max_2;

   void ChangeColor( TransferFunction2D input_tfunc );

   void setLimit(float cut_sei_min, float cut_sei_max, float cut_gt_min, float cut_gt_max);

   void updateHistogram(void);

   std::vector<unsigned int> surface_node;

   public slots:
   void ChangeTimeSlider(int value);
   void updateHistogramUsingLog(float bias_parameter);
   void Reset_Rendering();
 };

 #endif
