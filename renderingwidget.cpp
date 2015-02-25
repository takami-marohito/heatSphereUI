#include "renderingwidget.h"

#include "Q.h"
//#include "Qedge.h"

#include <kvs/qt/Screen>
#include <kvs/glut/Screen>
#include <QGridLayout>
#include <string.h>

#include <kvs/StructuredVolumeObject>
#include <kvs/StructuredVolumeImporter>
#include <kvs/RayCastingRenderer>

#include "loaducd.h"
#include "value_structure.h"
#include <kvs/TransferFunction>
#include <kvs/OpacityMap>

#include <kvs/glut/Label>

#include <kvs/ObjectManager>
#include <kvs/RendererManager>

#include <limits.h>

#include <kvs/RGBFormulae>

#include <math.h>

#include <kvs/TargetChangeEvent>
#include <kvs/Light>

#include <chrono>

#include <kvs/StochasticPolygonRenderer>
#include <kvs/StochasticRenderingCompositor>
#include <kvs/PolygonObject>

#include "CalcLengthSecondInvariant.h"
#include "CalcGradT.h"
#include "CalcLengthGradT.h"
#include "GradTzdir.h"

#define TETRA 4
#define PRISM 6

namespace
{
  float base_opacity = 0.6;
  //int subpixel = 10;
  int rep = 36;
  std::string object_name = "Object";
  std::string renderer_name = "Renderer";
  int RenderingWindowX = 800;
  int RenderingWindowY = 800;
  float th_length_sei = 10;
}



unsigned int RenderingWidget::SecondInvariantColor(float secondinvariant)
{
  float cut_sei = secondinvariant; 
  if(secondinvariant>cut_value_sei_max){
    cut_sei = cut_value_sei_max;
  }
  if(secondinvariant<cut_value_sei_min){
    cut_sei = cut_value_sei_min;
  }
  cut_sei = (cut_sei-cut_value_sei_min)/(cut_value_sei_max-cut_value_sei_min);
  unsigned int value = round(cut_sei*255.0);
  return(value);
}


unsigned int RenderingWidget::GradTColor(float gradt)
{
  float cut_gt = gradt; 
  if(cut_gt>cut_value_gt_max){
    cut_gt = cut_value_gt_max;
  }
  if(cut_gt<cut_value_gt_min){
    cut_gt = cut_value_gt_min;
  }
  cut_gt = (cut_gt-cut_value_gt_min)/(cut_value_gt_max-cut_value_gt_min);
  unsigned int value = round(cut_gt*255.0);
  return(value);
}

float RenderingWidget::CutSecondInvariant(float secondinvariant)
{
  float cut_sei = secondinvariant; 
  if(secondinvariant>cut_value_sei_max){
    cut_sei = cut_value_sei_max;
  }
  if(secondinvariant<cut_value_sei_min){
    cut_sei = cut_value_sei_min;
  }
  return(cut_sei);
}

float RenderingWidget::CutGradT(float gradt)
{
  float cut_gt = gradt; 
  if(cut_gt>cut_value_gt_max){
    cut_gt = cut_value_gt_max;
  }
  if(cut_gt<cut_value_gt_min){
    cut_gt = cut_value_gt_min;
  }
  return(cut_gt);
}

float RenderingWidget::HistogramCutSecondInvariant(float secondinvariant)
{
  float cut_sei = secondinvariant; 
  if(secondinvariant>histogram_cut_value_sei_max){
    cut_sei = histogram_cut_value_sei_max;
  }
  if(secondinvariant<histogram_cut_value_sei_min){
    cut_sei = histogram_cut_value_sei_min;
  }
  return(cut_sei);
}

float RenderingWidget::HistogramCutGradT(float gradt)
{
  float cut_gt = gradt; 
  if(cut_gt>histogram_cut_value_gt_max){
    cut_gt = histogram_cut_value_gt_max;
  }
  if(cut_gt<histogram_cut_value_gt_min){
    cut_gt = histogram_cut_value_gt_min;
  }
  return(cut_gt);
}


float RenderingWidget::LengthAandB(float ax, float ay, float az, float bx, float by, float bz)
{
  float length = (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz);
  return(sqrt(length));
}


RenderingWidget::RenderingWidget(kvs::qt::Application* app):r(0.0),theta(0.78),phi(0.78),spot_flag(true),cut_value_sei_min(22.35),cut_value_gt_min(22.35),cut_value_sei_max(24),cut_value_gt_max(24),histogram_cut_value_sei_min(22.35),histogram_cut_value_gt_min(22.35),histogram_cut_value_sei_max(24),histogram_cut_value_gt_max(24)
{
  m_screen = new kvs::qt::Screen( app );
  m_screen->setFixedSize(RenderingWindowX,RenderingWindowY);
  QGridLayout *Layout = new QGridLayout();
  Layout->addWidget( m_screen, 0, 0 );
  this->setLayout(Layout);
  //  m_screen->addEvent(&m_targetchangeevent);
  //m_screen->setControlTargetToLight();

  m_screen->show();
}

RenderingWidget::~RenderingWidget()
{
}

void RenderingWidget::open( std::string filename )
{   
  m_filename = filename;
  if ( m_screen->scene()->hasObject( object_name ) )
    {
      m_screen->scene()->removeObject( object_name );
    }
  auto start_time_total = std::chrono::system_clock::now();
  auto start_time_load = std::chrono::system_clock::now();
  
  kvs::UnstructuredVolumeObject* ucd = new takami::LoadUcd( filename.c_str(), PRISM );
  
  const size_t nnodes = ucd->numberOfNodes();
  const size_t ncells = ucd->numberOfCells();
  const kvs::ValueArray<kvs::Real32> coords = ucd->coords();
  const kvs::ValueArray<kvs::UInt32> connections = ucd->connections();
  delete ucd;
  
  kvs::UnstructuredVolumeObject* vel_u = new takami::LoadUcd( filename.c_str(), PRISM, 5);
  kvs::UnstructuredVolumeObject* vel_v = new takami::LoadUcd( filename.c_str(), PRISM, 6);
  kvs::UnstructuredVolumeObject* vel_w = new takami::LoadUcd( filename.c_str(), PRISM, 7);

  const float *buf_u = static_cast<const float*>(vel_u->values().data());
  const float *buf_v = static_cast<const float*>(vel_v->values().data());
  const float *buf_w = static_cast<const float*>(vel_w->values().data());
  
  kvs::ValueArray<kvs::Real32> kvsValuesOfVelocity(vel_u->numberOfNodes() * 3);
  kvs::Real32* pkvsValuesOfVelocity = kvsValuesOfVelocity.data();
  for(size_t i=0;i<vel_u->numberOfNodes();i++){
    *(pkvsValuesOfVelocity++) = buf_u[i];
    *(pkvsValuesOfVelocity++) = buf_v[i];
    *(pkvsValuesOfVelocity++) = buf_w[i];
  }

  kvs::UnstructuredVolumeObject* objectForSei = new kvs::UnstructuredVolumeObject();
  
  objectForSei->setCellTypeToPrism();
  objectForSei->setNumberOfNodes( nnodes );
  objectForSei->setNumberOfCells( ncells );
  objectForSei->setVeclen( 3 );
  objectForSei->setCoords( coords );
  objectForSei->setConnections( connections );
  objectForSei->setValues(kvs::AnyValueArray( kvsValuesOfVelocity ) );

  //objectForSei->setMinMaxObjectCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );
  //objectForSei->setMinMaxExternalCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );

  object1 = new local::Q( objectForSei );

  std::vector<float> SecondInvariant(object1->numberOfNodes(),0);

  const float* buf = static_cast<const float*>(object1->values().data());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    SecondInvariant.at(i) = buf[i];
  }
  kvs::UnstructuredVolumeObject* temperature = new takami::LoadUcd(filename.c_str(), PRISM, 3);

  kvs::UnstructuredVolumeObject* GradT = new local::GradTzdir( temperature );

  m_SecondInvariant.resize(SecondInvariant.size(),0);
  std::copy(SecondInvariant.begin(), SecondInvariant.end(), m_SecondInvariant.begin());

  surface_polygon_connection.resize(0);
  for(unsigned int i=0;i<object1->numberOfCells();i++){
    for(unsigned int j=0;j<6;j++){
      if(LengthAandB(object1->coords()[object1->connections()[i*6+j]*3+0], object1->coords()[object1->connections()[i*6+j]*3+1], object1->coords()[object1->connections()[i*6+j]*3+2], 0,0,0)  < 22.355 ){
	for(unsigned int k=0;k<6;k++){
	  surface_polygon_connection.push_back(object1->connections()[i*6+k]);
	}
	break;
      }
    }
  }

  surface_node.resize(0);
  std::vector<bool> histogram_connection_flag(object1->numberOfNodes(),false);
  for(unsigned int i=0;i<surface_polygon_connection.size()/6;i++){
    for(unsigned int j=0;j<3;j++){
      unsigned int index = surface_polygon_connection.at(i*6+j);
      if(histogram_connection_flag.at(index) == false){
	histogram_connection_flag.at(index)=true;
	surface_node.push_back(index);
      }
    }
  }
  /*
  kvs::ValueArray<kvs::Real32> value1(&SecondInvariant.at(0), SecondInvariant.size() );
  object1->setValues( value1 );
  object1->updateMinMaxValues();
  */


  datastruct1.value.resize(object1->numberOfNodes());
  //datastruct1.value = CalcSecondInvariant_Prism(object1, vel_u, vel_v, vel_w);
  datastruct1.value = CalcLengthSecondInvariant(object1, surface_node, SecondInvariant, th_length_sei);
  datastruct1.name = "Length Second Invariant";

  const float* buf_gradT = static_cast<const float*>(GradT->values().data());
  std::vector<float> vector_gradT(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    vector_gradT.at(i) = buf_gradT[i];
  }

  datastruct2.value.resize(object1->numberOfNodes());
  datastruct2.value = CalcLengthGradT(object1, surface_node, vector_gradT);
  //datastruct2.value = CalcAbsGradT(object1, temperature);
  datastruct2.name = "Length GradT";

  histogramdata1.value.resize(object1->numberOfNodes());
  histogramdata2.value.resize(object1->numberOfNodes());
  std::copy(datastruct1.value.begin(), datastruct1.value.end(), histogramdata1.value.begin());
  std::copy(datastruct2.value.begin(), datastruct2.value.end(), histogramdata2.value.begin());
  histogramdata1.name = "LengthSecondInvariant";
  histogramdata2.name = "LengthGradT";

  kvs::ValueArray<kvs::UInt32> kvsConnection(&surface_polygon_connection.at(0), surface_polygon_connection.size() );
  /*
  object1->setNumberOfCells(surface_polygon_connection.size()/6);
  object1->setConnections(kvsConnection);
  */

  auto end_time_load = std::chrono::system_clock::now();

  kvs::TransferFunction tfunc_triangle=kvs::TransferFunction(kvs::RGBFormulae::Rainbow(256));
  std::vector<unsigned char> triangle_color1;
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_color1.push_back(255);
    triangle_color1.push_back(0);
    triangle_color1.push_back(0);
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleColor1(&triangle_color1.at(0), triangle_color1.size() );

  std::vector<unsigned char> triangle_color2;
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_color2.push_back(0);
    triangle_color2.push_back(0);
    triangle_color2.push_back(255);
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleColor2(&triangle_color2.at(0), triangle_color2.size() );

  std::vector<unsigned char> triangle_opacity1(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity1.at(i) = (unsigned char)(SecondInvariantColor(datastruct1.value.at(i)));
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity1(&triangle_opacity1.at(0), triangle_opacity1.size() );

  std::vector<unsigned char> triangle_opacity2(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity2.at(i) = (unsigned char)(GradTColor(datastruct2.value.at(i)));
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity2(&triangle_opacity2.at(0), triangle_opacity2.size() );

  std::vector<unsigned int> triangle_connection;
  for(unsigned int i=0;i<surface_polygon_connection.size()/6;i++){
    for(unsigned int j=0;j<3;j++){
      triangle_connection.push_back(surface_polygon_connection.at(i*6+j));
    }
  }
  kvs::ValueArray<kvs::UInt32> kvsTriangleConnection(&triangle_connection.at(0), triangle_connection.size() );

  std::vector<float> normal;
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    float plength = LengthAandB(object1->coords()[i*3+0], object1->coords()[i*3+1], object1->coords()[i*3+2], 0,0,0);
    normal.push_back((1)*object1->coords()[i*3+0]/plength);
    normal.push_back((1)*object1->coords()[i*3+1]/plength);
    normal.push_back((1)*object1->coords()[i*3+2]/plength);
  }
  kvs::ValueArray<kvs::Real32> kvsNormal(&normal.at(0), normal.size() );

  //  create polygon1
  pobject1 = new kvs::PolygonObject();
  pobject1->setName( "Polygon1" );
  pobject1->setCoords( object1->coords() );
  pobject1->setColors( kvsTriangleColor1 );
  pobject1->setOpacities( kvsTriangleOpacity1 );
  pobject1->setNormals( kvsNormal );
  pobject1->setConnections( kvsTriangleConnection );
  pobject1->setPolygonType( kvs::PolygonObject::Triangle );
  pobject1->setColorType( kvs::PolygonObject::VertexColor );
  pobject1->setNormalType( kvs::PolygonObject::VertexNormal );

  pobject1->setMinMaxObjectCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );
  pobject1->setMinMaxExternalCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );

  //  create polygon2
  pobject2 = new kvs::PolygonObject();
  pobject2->setName( "Polygon2" );
  pobject2->setCoords( object1->coords() );
  pobject2->setColors( kvsTriangleColor2 );
  pobject2->setOpacities( kvsTriangleOpacity2 );
  pobject2->setNormals( kvsNormal );
  pobject2->setConnections( kvsTriangleConnection );
  pobject2->setPolygonType( kvs::PolygonObject::Triangle );
  pobject2->setColorType( kvs::PolygonObject::VertexColor );
  pobject2->setNormalType( kvs::PolygonObject::VertexNormal );

  pobject2->setMinMaxObjectCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );
  pobject2->setMinMaxExternalCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );

  // create background poygon
  pobject_background = new kvs::PolygonObject();
  pobject_background->setName( "PolygonBackground" );
  std::vector<float> bcoord(object1->numberOfNodes()*3);
  for(unsigned int i=0;i<object1->numberOfNodes()*3;i++){
    bcoord.at(i) = object1->coords()[i] * 19.8 / 20.0;
  }
  kvs::ValueArray<kvs::Real32> kvsBcoord(&bcoord.at(0), bcoord.size() );
  std::vector<unsigned char> bcolor(object1->numberOfNodes()*3, 255);
  /*
  for(unsigned int i=0;i<triangle_connection.size();i++){
    if(spot_flag == true){
      unsigned int index = triangle_connection.at(i);
      if(LengthAandB(object1->coords()[index*3+0], object1->coords()[index*3+1], object1->coords()[index*3+2], 22.35*cos(phi)*cos(theta), 22.35*cos(phi)*sin(theta), 22.35*sin(phi)) < r){
	bcolor.at(index*3+0)=0;
	bcolor.at(index*3+2)=0;
      }
    }
  }
  */
  kvs::ValueArray<kvs::UInt8> kvsBcolor(&bcolor.at(0), bcolor.size() );
  std::vector<unsigned char> bopacity(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    bopacity.at(i) = 255;
  }
  kvs::ValueArray<kvs::UInt8> kvsBopacity(&bopacity.at(0), bopacity.size() );
  pobject_background->setCoords( kvsBcoord );
  pobject_background->setColors( kvsBcolor );
  pobject_background->setOpacities( kvsBopacity );
  pobject_background->setNormals( kvsNormal );
  pobject_background->setConnections( kvsTriangleConnection );
  pobject_background->setPolygonType( kvs::PolygonObject::Triangle );
  pobject_background->setColorType( kvs::PolygonObject::VertexColor );
  pobject_background->setNormalType( kvs::PolygonObject::VertexNormal );
  pobject_background->setMinMaxObjectCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );
  pobject_background->setMinMaxExternalCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );

  kvs::StochasticPolygonRenderer *prenderer1 = new kvs::StochasticPolygonRenderer();
  kvs::StochasticPolygonRenderer *prenderer2 = new kvs::StochasticPolygonRenderer();
  kvs::StochasticPolygonRenderer *prenderer_back = new kvs::StochasticPolygonRenderer();

  prenderer1->setShader(kvs::Shader::BlinnPhong(0.2,0.8,0.1,1) );
  prenderer2->setShader(kvs::Shader::BlinnPhong(0.2,0.8,0.1,1) );

  //kvs::glut::Screen *screen = static_cast<kvs::glut::Screen*>(m_screen);

  //m_screen->setTitle( "Polygon Object" );
  m_screen->registerObject( pobject_background, prenderer_back );
  m_screen->registerObject( pobject1, prenderer1 );
  m_screen->registerObject( pobject2, prenderer2 );
  //m_screen->scene()->camera()->setPosition(kvs::Vector3f(4,-4,4),kvs::Vector3f(-1,1,-1));
  //m_screen->scene()->light()->setPosition(kvs::Vector3f(6,-6,6));
  compositor = new kvs::StochasticRenderingCompositor( m_screen->scene() );
  compositor->setRepetitionLevel( 5 );
  compositor->enableLODControl();
  compositor->enableRefinement();
  m_screen->setEvent( compositor );
  //screen->setBackgroundColor(kvs::RGBColor(0,0,0));
  m_screen->show();



  this->CreateHistogram( histogramdata1.value, histogramdata2.value, surface_node );
  auto end_time_total = std::chrono::system_clock::now();
  auto diff_time_total = end_time_total-start_time_total;
  auto diff_time_load = end_time_load-start_time_load;
  std::cout << "load time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time_load).count() << "msec" << std::endl;
  std::cout << "total time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time_total).count() << "msec" << std::endl;
}

void RenderingWidget::open_sei( std::string filename, std::string filename_sei )
{   
}


void RenderingWidget::open_sei( std::string filename )
{   
}



void RenderingWidget::open_animation( std::vector<std::string> filename, int nsteps )
{   
}




void RenderingWidget::LoadHistogram(std::string filename)
{
  FILE *fp;
  fp = fopen("./histogram", "rb");
  histogram.resize(256*256);
  fread(&histogram.at(0), sizeof(int), 256*256, fp);
  fclose(fp);
  return;
}

void RenderingWidget::apply( TransferFunction2D input_tfunc )
{
  auto start_time = std::chrono::system_clock::now();

  this->ChangeColor(input_tfunc);

  auto end_time = std::chrono::system_clock::now();
  auto diff_time = end_time-start_time;
  std::cout << "apply done " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time).count() << "msec" << std::endl;
 }


void RenderingWidget::RenderingNthStep(int step)
{
}

void RenderingWidget::ChangeTimeSlider(int value)
{
  this->RenderingNthStep(value);
}

void RenderingWidget::CreateHistogramZooming(std::vector<float> &value1, std::vector<float> &value2, float f_startx, float f_starty, float f_vertical, float f_horizontal)
{
  //printf("zoom %f %f start\n",f_starty,f_vertical);
  float min_1 = tmp_min_1;
  float max_1 = tmp_max_1;
  float min_2 = tmp_min_2;
  float max_2 = tmp_max_2;
  histogram_zoom.resize(256*256);

  for(int i=0;i<256*256;i++){
    histogram_zoom.at(i) = 0;
  }
  
  float zoom_min_1 = (max_1-min_1)*f_startx + min_1;
  float zoom_max_1 = (max_1-min_1)*f_horizontal + zoom_min_1;
  float zoom_max_2 = (max_2-min_2)*(1-f_starty) + min_2;
  float zoom_min_2 = zoom_max_2-(max_2-min_2)*f_vertical;

  tmp_min_1 = zoom_min_1;
  tmp_max_1 = zoom_max_1;
  tmp_min_2 = zoom_min_2;
  tmp_max_2 = zoom_max_2;
  printf("zoom x from %f to %f, zoom y from %f to %f \n",tmp_min_1, tmp_max_1, tmp_min_2, tmp_max_2);
  for(unsigned  int i=0;i<surface_node.size();i++){
    if(HistogramCutSecondInvariant(value1.at(surface_node.at(i))) < zoom_min_1 || HistogramCutSecondInvariant(value1.at(surface_node.at(i))) > zoom_max_1){
    }else if(HistogramCutGradT(value2.at(surface_node.at(i))) < zoom_min_2 || HistogramCutGradT(value2.at(surface_node.at(i))) > zoom_max_2){
    }else{
      const int x = round( (HistogramCutSecondInvariant(value1.at(surface_node.at(i)))-zoom_min_1)/(zoom_max_1-zoom_min_1)*255.0 );
      const int y = round( (HistogramCutGradT(value2.at(surface_node.at(i)))-zoom_min_2)/(zoom_max_2-zoom_min_2)*255.0 );
      histogram_zoom.at(x+y*256)++;
    }
  }


  histogramForUpdate.resize(256*256);

  std::copy(histogram_zoom.begin(), histogram_zoom.end(), histogramForUpdate.begin());

  int max = *std::max_element( histogramForUpdate.begin(), histogramForUpdate.end() );
  
  for(long int i=0;i<256*256;i++){
    if(histogram_zoom.at(i)!=0){
      histogram_zoom.at(i) = log(histogram_zoom.at(i))+1;
    }
  }
  
  float min_histogram = *std::min_element( histogram_zoom.begin(), histogram_zoom.end() );
  float max_histogram = *std::max_element( histogram_zoom.begin(), histogram_zoom.end() );

  for(long int i=0;i<256*256;i++){
    histogram_zoom.at(i) = (int)((histogram_zoom.at(i)-min_histogram)/(max_histogram-min_histogram)*255);
  }

  std::vector<unsigned char> triangle_opacity1(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity1.at(i) = (unsigned char)(SecondInvariantColor(datastruct1.value.at(i)));
  }
  for(unsigned  int i=0;i<surface_node.size();i++){
    if(HistogramCutSecondInvariant(value1.at(surface_node.at(i))) < zoom_min_1 || HistogramCutSecondInvariant(value1.at(surface_node.at(i))) > zoom_max_1 || HistogramCutGradT(value2.at(surface_node.at(i))) < zoom_min_2 || HistogramCutGradT(value2.at(surface_node.at(i))) > zoom_max_2){
      triangle_opacity1.at(surface_node.at(i)) = 0;
    }
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity1(&triangle_opacity1.at(0), triangle_opacity1.size() );

  std::vector<unsigned char> triangle_opacity2(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity2.at(i) = (unsigned char)(GradTColor(datastruct2.value.at(i)));
  }
  for(unsigned  int i=0;i<surface_node.size();i++){
    if(HistogramCutSecondInvariant(value1.at(surface_node.at(i))) < zoom_min_1 || HistogramCutSecondInvariant(value1.at(surface_node.at(i))) > zoom_max_1 || HistogramCutGradT(value2.at(surface_node.at(i))) < zoom_min_2 || HistogramCutGradT(value2.at(surface_node.at(i))) > zoom_max_2){
      triangle_opacity2.at(surface_node.at(i)) = 0;
    }
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity2(&triangle_opacity2.at(0), triangle_opacity2.size() );

  std::vector<unsigned char> bopacity(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    bopacity.at(i) = 255;
  }
  for(unsigned  int i=0;i<surface_node.size();i++){
    if(HistogramCutSecondInvariant(value1.at(surface_node.at(i))) < zoom_min_1 || HistogramCutSecondInvariant(value1.at(surface_node.at(i))) > zoom_max_1 || HistogramCutGradT(value2.at(surface_node.at(i))) < zoom_min_2 || HistogramCutGradT(value2.at(surface_node.at(i))) > zoom_max_2){
      //bopacity.at(surface_node.at(i)) = 0;
    }
  }
  kvs::ValueArray<kvs::UInt8> kvsBOpacity(&bopacity.at(0), bopacity.size() );

  kvs::PolygonObject *pobjectcopy1 = new kvs::PolygonObject();
  pobjectcopy1->shallowCopy( *pobject1 );
  pobjectcopy1->setName("Polygon1");
  kvs::PolygonObject *pobjectcopy2 = new kvs::PolygonObject();
  pobjectcopy2->shallowCopy( *pobject2 );
  pobjectcopy2->setName("Polygon2");
  kvs::PolygonObject *pobjectcopy_background = new kvs::PolygonObject();
  pobjectcopy_background->shallowCopy( *pobject_background );
  pobjectcopy_background->setName("PolygonBackground");


  pobjectcopy1->setOpacities(kvsTriangleOpacity1);
  pobjectcopy2->setOpacities(kvsTriangleOpacity2);
  pobjectcopy_background->setOpacities(kvsBOpacity);
  m_screen->scene()->objectManager()->change("Polygon1", pobjectcopy1,false);
  m_screen->scene()->objectManager()->change("Polygon2", pobjectcopy2,false);
  m_screen->scene()->objectManager()->change("PolygonBackground", pobjectcopy_background,false);
  m_screen->redraw();

  return;
}

void RenderingWidget::updateHistogramUsingLog(float bias_parameter)
{
  int max = *std::max_element( histogramForUpdate.begin(), histogramForUpdate.end() );

  for(int i=0;i<256*256;i++){
    float f = (float)(histogramForUpdate.at(i)) / (float)(max);
    float b = std::pow( f, static_cast<float>(std::log(bias_parameter)/std::log(0.05)) );
    histogram.at(i) = 255*b;
  }
  /*
  FILE *fp;
  fp = fopen("./histogram", "wb");
  fwrite(&histogram.at(0), sizeof(int), 256*256, fp);
  fclose(fp);
  */
  return;
}

void RenderingWidget::updateHistogram(void)
{
  histogram_zoom.resize(256*256);

  for(int i=0;i<256*256;i++){
    histogram_zoom.at(i) = 0;
  }
  
  std::vector<unsigned int> surface_node_area;

  if(r!=0){
  for(unsigned  int i=0;i<surface_node.size();i++){
    if(HistogramCutSecondInvariant(datastruct1.value.at(surface_node.at(i))) < tmp_min_1 || HistogramCutSecondInvariant(datastruct1.value.at(surface_node.at(i))) > tmp_max_1){
    }else if(HistogramCutGradT(datastruct2.value.at(surface_node.at(i))) < tmp_min_2 || HistogramCutGradT(datastruct2.value.at(surface_node.at(i))) > tmp_max_2){
    }else if(LengthAandB(object1->coords()[surface_node.at(i)*3+0], object1->coords()[surface_node.at(i)*3+1], object1->coords()[surface_node.at(i)*3+2], 22.35*cos(phi)*cos(theta), 22.35*cos(phi)*sin(theta), 22.35*sin(phi)) < r){
      const int x = round( (HistogramCutSecondInvariant(datastruct1.value.at(surface_node.at(i)))-tmp_min_1)/(tmp_max_1-tmp_min_1)*255.0 );
      const int y = round( (HistogramCutGradT(datastruct2.value.at(surface_node.at(i)))-tmp_min_2)/(tmp_max_2-tmp_min_2)*255.0 );
      histogram_zoom.at(x+y*256)++;
      surface_node_area.push_back(surface_node.at(i));
    }
  }
  }else{
  for(unsigned  int i=0;i<surface_node.size();i++){
    if(HistogramCutSecondInvariant(datastruct1.value.at(surface_node.at(i))) < tmp_min_1 || HistogramCutSecondInvariant(datastruct1.value.at(surface_node.at(i))) > tmp_max_1){
    }else if(HistogramCutGradT(datastruct2.value.at(surface_node.at(i))) < tmp_min_2 || HistogramCutGradT(datastruct2.value.at(surface_node.at(i))) > tmp_max_2){
    }else{
      const int x = round( (HistogramCutSecondInvariant(datastruct1.value.at(surface_node.at(i)))-tmp_min_1)/(tmp_max_1-tmp_min_1)*255.0 );
      const int y = round( (HistogramCutGradT(datastruct2.value.at(surface_node.at(i)))-tmp_min_2)/(tmp_max_2-tmp_min_2)*255.0 );
      histogram_zoom.at(x+y*256)++;
      surface_node_area.push_back(surface_node.at(i));
    }
  }
  }

  printf("avg length %lf\n", CalcTimesLength(datastruct1.value, datastruct2.value, surface_node_area) );

  histogramForUpdate.resize(256*256);

  std::copy(histogram_zoom.begin(), histogram_zoom.end(), histogramForUpdate.begin());

  int max = *std::max_element( histogramForUpdate.begin(), histogramForUpdate.end() );
  
  for(long int i=0;i<256*256;i++){
    if(histogram_zoom.at(i)!=0){
      histogram_zoom.at(i) = log(histogram_zoom.at(i))+1;
    }
  }
  
  float min_histogram = *std::min_element( histogram_zoom.begin(), histogram_zoom.end() );
  float max_histogram = *std::max_element( histogram_zoom.begin(), histogram_zoom.end() );

  for(long int i=0;i<256*256;i++){
    histogram_zoom.at(i) = (int)((histogram_zoom.at(i)-min_histogram)/(max_histogram-min_histogram)*255);
  }
  return;
}

void RenderingWidget::CreateHistogram(std::vector<float> &value1, std::vector<float> &value2, std::vector<unsigned int> &index_matrix)
{
  float min_1 = 10000;
  float max_1 = -10000;
  float min_2 = 10000;
  float max_2 = -10000;

  for(unsigned int i=0;i<index_matrix.size();i++){
    if(min_1 > HistogramCutSecondInvariant(value1.at(index_matrix.at(i)))){
      min_1 = HistogramCutSecondInvariant(value1.at(index_matrix.at(i)));
    }
    if(max_1 < HistogramCutSecondInvariant(value1.at(index_matrix.at(i)))){
      max_1 = HistogramCutSecondInvariant(value1.at(index_matrix.at(i)));
    }
    if(min_2 > HistogramCutGradT(value2.at(index_matrix.at(i)))){
      min_2 = HistogramCutGradT(value2.at(index_matrix.at(i)));
    }
    if(max_2 < HistogramCutGradT(value2.at(index_matrix.at(i)))){
      max_2 = HistogramCutGradT(value2.at(index_matrix.at(i)));
    }
  }
  tmp_min_1 = min_1;
  tmp_min_2 = min_2;
  tmp_max_1 = max_1;
  tmp_max_2 = max_2;

  histogram.resize(256*256);

  for(int x=0;x<256;x++){
    for(int y=0;y<256;y++){
      histogram.at(x+y*256) = 0;
    }
  }
  histogramForUpdate.resize(256*256);

  for(long int i=0;i<index_matrix.size();i++){
    const int x = round( (HistogramCutSecondInvariant(value1.at(index_matrix.at(i)))-min_1)/(max_1-min_1)*255.0 );
    const int y = round( (HistogramCutGradT(value2.at(index_matrix.at(i)))-min_2)/(max_2-min_2)*255.0 );
    histogram.at(x+y*256)++;
  }

  std::copy(histogram.begin(), histogram.end(), histogramForUpdate.begin());

  int max = *std::max_element( histogramForUpdate.begin(), histogramForUpdate.end() );
  
  for(long int i=0;i<256*256;i++){
    if(histogram.at(i)!=0){
      histogram.at(i) = log(histogram.at(i))+1;
    }
  }

  //端のところをなくしてもっと特徴が出やすくする

  for(long int i=0;i<256;i++){
    for(long int j=0;j<256;j++){
      if(i==0 || j==0){
	histogram.at(i) = 1;
      }
    }
  }

  float min_histogram = *std::min_element( histogram.begin(), histogram.end() );
  float max_histogram = *std::max_element( histogram.begin(), histogram.end() );

  for(long int i=0;i<256*256;i++){
    histogram.at(i) = (int)((histogram.at(i)-min_histogram)/(max_histogram-min_histogram)*255);
  }

  /*
  std::vector<unsigned int> distribution(256,0);
  for(unsigned int i=0;i<256;i++){
    for(unsigned int j=0;j<256;j++){
      distribution.at(i)+=histogram.at(j*256+i);
    }
    printf("%u\n",distribution.at(i));
  }
  */

  return;
}


void RenderingWidget::Reset_Rendering()
{
  r = 0;
  theta = 0.78;
  phi = 0.78;
  std::vector<unsigned char> triangle_opacity1(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity1.at(i) = (unsigned char)(SecondInvariantColor(datastruct1.value.at(i)));
  }

  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity1(&triangle_opacity1.at(0), triangle_opacity1.size() );
  std::vector<unsigned char> triangle_opacity2(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity2.at(i) = (unsigned char)(GradTColor(datastruct2.value.at(i)));
  }

  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity2(&triangle_opacity2.at(0), triangle_opacity2.size() );
  std::vector<unsigned char> bopacity(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    bopacity.at(i) = 255;
  }
  kvs::ValueArray<kvs::UInt8> kvsBOpacity(&bopacity.at(0), bopacity.size() );
  kvs::PolygonObject *pobjectcopy1 = new kvs::PolygonObject();
  pobjectcopy1->shallowCopy( *pobject1 );
  pobjectcopy1->setName("Polygon1");
  kvs::PolygonObject *pobjectcopy2 = new kvs::PolygonObject();
  pobjectcopy2->shallowCopy( *pobject2 );
  pobjectcopy2->setName("Polygon2");
  kvs::PolygonObject *pobjectcopy_background = new kvs::PolygonObject();
  std::cout << "reset rendering" << std::endl;
  pobjectcopy_background->shallowCopy( *pobject_background );
  pobjectcopy_background->setName("PolygonBackground");
  pobjectcopy1->setOpacities(kvsTriangleOpacity1);
  pobjectcopy2->setOpacities(kvsTriangleOpacity2);
  pobjectcopy_background->setOpacities(kvsBOpacity);
  m_screen->scene()->replaceObject("Polygon1", pobjectcopy1,false);
  m_screen->scene()->replaceObject("Polygon2", pobjectcopy2,false);
  m_screen->scene()->replaceObject("PolygonBackground", pobjectcopy_background,false);
  m_screen->redraw();
  std::cout << "reset finish" << std::endl;
  return;
}


void RenderingWidget::CreateHistogramZooming(std::vector<float> &value1, std::vector<float> &value2, float inr, float intheta, float inphi)
{
  std::cout << "Clip Area R=" <<inr<<" Theta="<<intheta<<" Phi="<<inphi <<std::endl;

  if( inr == 0 ){
    std::cout << "r must be more than 0" << std::endl;
    return;
  }

  r = inr;
  theta = intheta;
  phi = inphi;

  histogram_zoom.resize(256*256);

  for(int i=0;i<256*256;i++){
    histogram_zoom.at(i) = 0;
  }
  
  std::vector<unsigned int> surface_node_area;

  for(unsigned  int i=0;i<surface_node.size();i++){
    if(HistogramCutSecondInvariant(value1.at(surface_node.at(i))) < tmp_min_1 || HistogramCutSecondInvariant(value1.at(surface_node.at(i))) > tmp_max_1){
    }else if(HistogramCutGradT(value2.at(surface_node.at(i))) < tmp_min_2 || HistogramCutGradT(value2.at(surface_node.at(i))) > tmp_max_2){
    }else if(LengthAandB(object1->coords()[surface_node.at(i)*3+0], object1->coords()[surface_node.at(i)*3+1], object1->coords()[surface_node.at(i)*3+2], 22.35*cos(phi)*cos(theta), 22.35*cos(phi)*sin(theta), 22.35*sin(phi)) < r){
      const int x = round( (HistogramCutSecondInvariant(value1.at(surface_node.at(i)))-tmp_min_1)/(tmp_max_1-tmp_min_1)*255.0 );
      const int y = round( (HistogramCutGradT(value2.at(surface_node.at(i)))-tmp_min_2)/(tmp_max_2-tmp_min_2)*255.0 );
      histogram_zoom.at(x+y*256)++;
      surface_node_area.push_back(surface_node.at(i));
    }
  }

  printf("avg length %lf\n", CalcTimesLength(datastruct1.value, datastruct2.value, surface_node_area) );

  histogramForUpdate.resize(256*256);

  std::copy(histogram_zoom.begin(), histogram_zoom.end(), histogramForUpdate.begin());

  int max = *std::max_element( histogramForUpdate.begin(), histogramForUpdate.end() );
  
  for(long int i=0;i<256*256;i++){
    if(histogram_zoom.at(i)!=0){
      histogram_zoom.at(i) = log(histogram_zoom.at(i))+1;
    }
  }
  
  float min_histogram = *std::min_element( histogram_zoom.begin(), histogram_zoom.end() );
  float max_histogram = *std::max_element( histogram_zoom.begin(), histogram_zoom.end() );

  for(long int i=0;i<256*256;i++){
    histogram_zoom.at(i) = (int)((histogram_zoom.at(i)-min_histogram)/(max_histogram-min_histogram)*255);
  }

  std::vector<unsigned char> triangle_opacity1(object1->numberOfNodes(),0);
  for(unsigned int i=0;i<surface_node_area.size();i++){
    triangle_opacity1.at(surface_node_area.at(i)) = (unsigned char)(SecondInvariantColor(datastruct1.value.at(surface_node_area.at(i))));
  }
  /*
  for(unsigned int i=0;i<surface_node.size();i++){
    triangle_opacity1.at(surface_node.at(i)) = (unsigned char)(SecondInvariantColor(datastruct1.value.at(surface_node.at(i))));
  }
  */

  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity1(&triangle_opacity1.at(0), triangle_opacity1.size() );

  std::vector<unsigned char> triangle_opacity2(object1->numberOfNodes(),0);
  for(unsigned int i=0;i<surface_node_area.size();i++){
    triangle_opacity2.at(surface_node_area.at(i)) = (unsigned char)(GradTColor(datastruct2.value.at(surface_node_area.at(i))));
  }
  /*
  for(unsigned int i=0;i<surface_node.size();i++){
    triangle_opacity2.at(surface_node.at(i)) = (unsigned char)(GradTColor(datastruct2.value.at(surface_node.at(i))));
  }
  */
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity2(&triangle_opacity2.at(0), triangle_opacity2.size() );

  std::vector<unsigned char> bopacity(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    bopacity.at(i) = 255;
  }
  kvs::ValueArray<kvs::UInt8> kvsBOpacity(&bopacity.at(0), bopacity.size() );

  std::vector<unsigned char> bcolor(object1->numberOfNodes()*3);
  for(unsigned int i=0;i<object1->numberOfNodes()*3;i++){
    bcolor.at(i) = 255;
  }
  for(unsigned  int i=0;i<surface_node.size();i++){
    if(LengthAandB(object1->coords()[surface_node.at(i)*3+0], object1->coords()[surface_node.at(i)*3+1], object1->coords()[surface_node.at(i)*3+2], 22.35*cos(phi)*cos(theta), 22.35*cos(phi)*sin(theta), 22.35*sin(phi)) < r){
      bcolor.at(surface_node.at(i)*3+0) = 150;
      bcolor.at(surface_node.at(i)*3+2) = 150;
    }
  }
  kvs::ValueArray<kvs::UInt8> kvsBColor(&bcolor.at(0), bcolor.size() );
 
  kvs::PolygonObject *pobjectcopy1 = new kvs::PolygonObject();
  pobjectcopy1->shallowCopy( *pobject1 );
  pobjectcopy1->setName("Polygon1");
  kvs::PolygonObject *pobjectcopy2 = new kvs::PolygonObject();
  pobjectcopy2->shallowCopy( *pobject2 );
  pobjectcopy2->setName("Polygon2");
  kvs::PolygonObject *pobjectcopy_background = new kvs::PolygonObject();
  pobjectcopy_background->shallowCopy( *pobject_background );
  pobjectcopy_background->setName("PolygonBackground");


  pobjectcopy1->setOpacities(kvsTriangleOpacity1);
  pobjectcopy2->setOpacities(kvsTriangleOpacity2);
  pobjectcopy_background->setOpacities(kvsBOpacity);
  pobjectcopy_background->setColors( kvsBColor );
  m_screen->scene()->objectManager()->change("Polygon1", pobjectcopy1,false);
  m_screen->scene()->objectManager()->change("Polygon2", pobjectcopy2,false);
  m_screen->scene()->objectManager()->change("PolygonBackground", pobjectcopy_background,false);
  m_screen->redraw();

  std::vector<unsigned int>().swap(surface_node_area);
  return;
}

void RenderingWidget::setLimit(float cut_sei_min, float cut_sei_max, float cut_gt_min, float cut_gt_max)
{
  cut_value_sei_min=cut_sei_min;
  cut_value_gt_min=cut_gt_min;
  cut_value_sei_max=cut_sei_max;
  cut_value_gt_max=cut_gt_max;
  return;
}


void RenderingWidget::ChangeColor( TransferFunction2D input_tfunc )
{
  std::cout << "color start" << std::endl;


  std::vector<unsigned int> surface_node_area;

  histogram_zoom.resize(256*256);

  for(int i=0;i<256*256;i++){
    histogram_zoom.at(i) = 0;
  }

  for(unsigned  int i=0;i<surface_node.size();i++){
    if(CutSecondInvariant(datastruct1.value.at(surface_node.at(i))) < tmp_min_1 || CutSecondInvariant(datastruct1.value.at(surface_node.at(i))) > tmp_max_1){
    }else if(CutGradT(datastruct2.value.at(surface_node.at(i))) < tmp_min_2 || CutGradT(datastruct2.value.at(surface_node.at(i))) > tmp_max_2){
    }else if(LengthAandB(object1->coords()[surface_node.at(i)*3+0], object1->coords()[surface_node.at(i)*3+1], object1->coords()[surface_node.at(i)*3+2], 22.35*cos(phi)*cos(theta), 22.35*cos(phi)*sin(theta), 22.35*sin(phi)) < r){
      const int x = round( (CutSecondInvariant(datastruct1.value.at(surface_node.at(i)))-tmp_min_1)/(tmp_max_1-tmp_min_1)*255.0 );
      const int y = round( (CutGradT(datastruct2.value.at(surface_node.at(i)))-tmp_min_2)/(tmp_max_2-tmp_min_2)*255.0 );
      histogram_zoom.at(x+y*256)++;
      surface_node_area.push_back(surface_node.at(i));
    }
  }


  histogramForUpdate.resize(256*256);

  std::copy(histogram_zoom.begin(), histogram_zoom.end(), histogramForUpdate.begin());

  int max = *std::max_element( histogramForUpdate.begin(), histogramForUpdate.end() );
  
  for(long int i=0;i<256*256;i++){
    if(histogram_zoom.at(i)!=0){
      histogram_zoom.at(i) = log(histogram_zoom.at(i))+1;
    }
  }
  

  float min_histogram = *std::min_element( histogram_zoom.begin(), histogram_zoom.end() );
  float max_histogram = *std::max_element( histogram_zoom.begin(), histogram_zoom.end() );

  for(long int i=0;i<256*256;i++){
    histogram_zoom.at(i) = (int)((histogram_zoom.at(i)-min_histogram)/(max_histogram-min_histogram)*255);
  }

  float step1 =  (tmp_max_1-tmp_min_1)/255.0;
  float step2 =  (tmp_max_2-tmp_min_2)/255.0;

  //triangle_opacity1のみで2次元伝達関数を反映する
  std::vector<unsigned char> triangle_opacity1(object1->numberOfNodes(),0);
  std::vector<unsigned char> triangle_color1(object1->numberOfNodes()*3,0);
  std::vector<unsigned char> triangle_opacity2(object1->numberOfNodes(),0);

  for(unsigned int i=0;i<surface_node_area.size();i++){
    float mod1 = fmod(CutSecondInvariant(datastruct1.value.at(surface_node_area.at(i)))-tmp_min_1, step1);
    int  quotient1 = (int)(round((CutSecondInvariant(datastruct1.value.at(surface_node_area.at(i)))-mod1) / step1) );
    float mod2 = fmod(CutSecondInvariant(datastruct2.value.at(surface_node_area.at(i)))-tmp_min_2, step2);
    int  quotient2 = (int)(round((CutGradT(datastruct2.value.at(surface_node_area.at(i)))-mod2) / step2) );
    QColor color = input_tfunc.cmap.at(quotient2*256+quotient1);
    float opacity = input_tfunc.omap.at(quotient2*256+quotient1);

    triangle_opacity1.at(surface_node_area.at(i)) = (unsigned char)(opacity);
    triangle_color1.at(surface_node_area.at(i)*3+0) = color.red();
    triangle_color1.at(surface_node_area.at(i)*3+1) = color.green();
    triangle_color1.at(surface_node_area.at(i)*3+2) = color.blue();
  }

  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity1(&triangle_opacity1.at(0), triangle_opacity1.size() );
  kvs::ValueArray<kvs::UInt8> kvsTriangleColor1(&triangle_color1.at(0), triangle_color1.size() );
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity2(&triangle_opacity2.at(0), triangle_opacity2.size() );

  std::vector<unsigned char> bopacity(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    bopacity.at(i) = 255;
  }
  kvs::ValueArray<kvs::UInt8> kvsBOpacity(&bopacity.at(0), bopacity.size() );

  std::vector<unsigned char> bcolor(object1->numberOfNodes()*3);
  for(unsigned int i=0;i<object1->numberOfNodes()*3;i++){
    bcolor.at(i) = 255;
  }
  for(unsigned  int i=0;i<surface_node.size();i++){
    if(LengthAandB(object1->coords()[surface_node.at(i)*3+0], object1->coords()[surface_node.at(i)*3+1], object1->coords()[surface_node.at(i)*3+2], 22.35*cos(phi)*cos(theta), 22.35*cos(phi)*sin(theta), 22.35*sin(phi)) < r){
      bcolor.at(surface_node.at(i)*3+0) = 150;
      bcolor.at(surface_node.at(i)*3+2) = 150;
    }
  }
  kvs::ValueArray<kvs::UInt8> kvsBColor(&bcolor.at(0), bcolor.size() );
 
  kvs::PolygonObject *pobjectcopy1 = new kvs::PolygonObject();
  pobjectcopy1->shallowCopy( *pobject1 );
  pobjectcopy1->setName("Polygon1");

  kvs::PolygonObject *pobjectcopy2 = new kvs::PolygonObject();
  pobjectcopy2->shallowCopy( *pobject2 );
  pobjectcopy2->setName("Polygon2");

  kvs::PolygonObject *pobjectcopy_background = new kvs::PolygonObject();
  pobjectcopy_background->shallowCopy( *pobject_background );
  pobjectcopy_background->setName("PolygonBackground");


  pobjectcopy1->setOpacities(kvsTriangleOpacity1);
  pobjectcopy1->setColors(kvsTriangleColor1);

  pobjectcopy2->setOpacities(kvsTriangleOpacity2);

  pobjectcopy_background->setOpacities(kvsBOpacity);
  pobjectcopy_background->setColors( kvsBColor );

  m_screen->scene()->objectManager()->change("Polygon1", pobjectcopy1,false);
  m_screen->scene()->objectManager()->change("Polygon2", pobjectcopy2,false);
  m_screen->scene()->objectManager()->change("PolygonBackground", pobjectcopy_background,false);
  m_screen->redraw();

  std::vector<unsigned int>().swap(surface_node_area);
  return;
}


double RenderingWidget::CalcTimesLength(std::vector<float> vec1, std::vector<float> vec2, std::vector<unsigned int> surface_node_area)
{
  float avg1 = 0;
  float avg2 = 0;

  float count = 0;

  for(unsigned int i=0;i<surface_node_area.size();i++){
    if(vec1.at(surface_node_area.at(i))>22){
      avg1 += vec1.at(surface_node_area.at(i));
      avg2 += vec2.at(surface_node_area.at(i));
      count++;
    }
  }
  avg1 = avg1 / count;
  avg2 = avg2 / count;

  printf("avg is %f %f size is %f %u\n",avg1, avg2, count, surface_node_area.size());

  std::vector<float> lvec1(surface_node_area.size(), 0);
  std::vector<float> lvec2(surface_node_area.size(), 0);

  for(unsigned int i=0;i<surface_node_area.size();i++){
    if(vec1.at(surface_node_area.at(i))>22){
      lvec1.at(i) = vec1.at(surface_node_area.at(i)) - avg1;
      lvec2.at(i) = vec2.at(surface_node_area.at(i)) - avg2;
    }
  }

  double cos = 0;
  double std_dev1 = 0;
  double std_dev2 = 0;

  for(unsigned int i=0;i<surface_node_area.size();i++){
    if(vec1.at(surface_node_area.at(i))>22){
      std_dev1 += lvec1.at(i)*lvec1.at(i);
      std_dev2 += lvec2.at(i)*lvec2.at(i);
    }
  }
  std_dev1 = sqrt(std_dev1);
  std_dev2 = sqrt(std_dev2);

  for(unsigned int i=0;i<surface_node_area.size();i++){
    if(vec1.at(surface_node_area.at(i))>22){
      cos += lvec1.at(i) * lvec2.at(i);
    }
  }
  return(cos/(std_dev1*std_dev2));

}


void RenderingWidget::open_another_time( std::string filename )
{   
  auto start_time_total = std::chrono::system_clock::now();
  auto start_time_load = std::chrono::system_clock::now();

  std::vector<float> temperature = takami::LoadUcdValue( filename.c_str(), 3);

  std::vector<float> GradT = CalcAbsGradT(object1, temperature);
  //std::vector<float> LengthGradT = CalcLengthGradT(object1, surface_node, GradT);
  printf("load %s for Length of GradT\n", filename.c_str());
  datastruct2.value = CalcLengthGradT(object1, surface_node, GradT);

  histogramdata2.value.resize(object1->numberOfNodes());
  std::copy(datastruct2.value.begin(), datastruct2.value.end(), histogramdata2.value.begin());
  histogramdata1.name = "LengthSecondInvariant";
  histogramdata2.name = "LengthGradT";

  auto end_time_load = std::chrono::system_clock::now();


  std::vector<unsigned char> triangle_opacity2(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity2.at(i) = (unsigned char)(GradTColor(datastruct2.value.at(i)));
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity2(&triangle_opacity2.at(0), triangle_opacity2.size() );

  kvs::PolygonObject *pobjectcopy2 = new kvs::PolygonObject();
  pobjectcopy2->shallowCopy( *pobject2 );
  pobjectcopy2->setName("Polygon2");
  pobjectcopy2->setOpacities(kvsTriangleOpacity2);
  m_screen->scene()->replaceObject("Polygon2", pobjectcopy2,false);
  m_screen->redraw();

  this->CreateHistogram( histogramdata1.value, histogramdata2.value, surface_node );
  auto end_time_total = std::chrono::system_clock::now();
  auto diff_time_total = end_time_total-start_time_total;
  auto diff_time_load = end_time_load-start_time_load;
  std::cout << "load time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time_load).count() << "msec" << std::endl;
  std::cout << "total time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time_total).count() << "msec" << std::endl;
}


void RenderingWidget::change_th_sei(float input)
{   
  auto start_time_total = std::chrono::system_clock::now();
  auto start_time_load = std::chrono::system_clock::now();

  printf("change the threshold of SecondInvariant %f\n",input);
  th_length_sei = input;

  datastruct1.value.resize(object1->numberOfNodes());
  //datastruct1.value = CalcSecondInvariant_Prism(object1, vel_u, vel_v, vel_w);
  datastruct1.value = CalcLengthSecondInvariant(object1, surface_node, m_SecondInvariant, th_length_sei);
  datastruct1.name = "Length Second Invariant";

  histogramdata1.value.resize(object1->numberOfNodes());
  std::copy(datastruct1.value.begin(), datastruct1.value.end(), histogramdata1.value.begin());
  histogramdata1.name = "LengthSecondInvariant";
  histogramdata2.name = "LengthGradT";

  auto end_time_load = std::chrono::system_clock::now();


  std::vector<unsigned char> triangle_opacity1(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity1.at(i) = (unsigned char)(SecondInvariantColor(datastruct1.value.at(i)));
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity1(&triangle_opacity1.at(0), triangle_opacity1.size() );

  kvs::PolygonObject *pobjectcopy1 = new kvs::PolygonObject();
  pobjectcopy1->shallowCopy( *pobject1 );
  pobjectcopy1->setName("Polygon1");
  pobjectcopy1->setOpacities(kvsTriangleOpacity1);
  m_screen->scene()->replaceObject("Polygon1", pobjectcopy1,false);
  m_screen->redraw();

  this->updateHistogram( );
  auto end_time_total = std::chrono::system_clock::now();
  auto diff_time_total = end_time_total-start_time_total;
  auto diff_time_load = end_time_load-start_time_load;
  std::cout << "load time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time_load).count() << "msec" << std::endl;
  std::cout << "total time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time_total).count() << "msec" << std::endl;
}

void RenderingWidget::open( std::string filename, std::string filename_sei )
{   
  m_filename = filename;
  if ( m_screen->scene()->hasObject( object_name ) )
    {
      m_screen->scene()->removeObject( object_name );
    }
  auto start_time_total = std::chrono::system_clock::now();
  auto start_time_load = std::chrono::system_clock::now();

  object1 = new kvs::UnstructuredVolumeObject();
  object1 = new takami::LoadUcd( filename.c_str(), PRISM, 0 );
  object1->setMinMaxObjectCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );
  object1->setMinMaxExternalCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );
  

  //std::vector<float> vel_u = takami::LoadUcdValue( filename.c_str(), 5);
  //std::vector<float> vel_v = takami::LoadUcdValue( filename.c_str(), 6);
  //std::vector<float> vel_w = takami::LoadUcdValue( filename.c_str(), 7);
  std::vector<float> temperature = takami::LoadUcdValue( filename.c_str(), 3);

  printf("loading SecondInvariant\n");
  FILE *fp3;
  fp3 = fopen(filename_sei.c_str(), "rb");
  int filesize;
  fread(&filesize, sizeof(int), 1, fp3);
  std::vector<float> SecondInvariant(filesize, 0);

  fread(&SecondInvariant.at(0), sizeof(float), filesize, fp3);
  fclose(fp3);
  //std::vector<float> SecondInvariant = CalcSecondInvariant_Prism(object1, vel_u, vel_v, vel_w);

  std::vector<float> GradT = CalcAbsGradT(object1, temperature);

  m_SecondInvariant.resize(SecondInvariant.size(),0);
  std::copy(SecondInvariant.begin(), SecondInvariant.end(), m_SecondInvariant.begin());

  surface_polygon_connection.resize(0);
  for(unsigned int i=0;i<object1->numberOfCells();i++){
    for(unsigned int j=0;j<6;j++){
      if(LengthAandB(object1->coords()[object1->connections()[i*6+j]*3+0], object1->coords()[object1->connections()[i*6+j]*3+1], object1->coords()[object1->connections()[i*6+j]*3+2], 0,0,0)  < 22.355 ){
	for(unsigned int k=0;k<6;k++){
	  surface_polygon_connection.push_back(object1->connections()[i*6+k]);
	}
	break;
      }
    }
  }

  surface_node.resize(0);
  std::vector<bool> histogram_connection_flag(object1->numberOfNodes(),false);
  for(unsigned int i=0;i<surface_polygon_connection.size()/6;i++){
    for(unsigned int j=0;j<3;j++){
      unsigned int index = surface_polygon_connection.at(i*6+j);
      if(histogram_connection_flag.at(index) == false){
	histogram_connection_flag.at(index)=true;
	surface_node.push_back(index);
      }
    }
  }
  /*
  kvs::ValueArray<kvs::Real32> value1(&SecondInvariant.at(0), SecondInvariant.size() );
  object1->setValues( value1 );
  object1->updateMinMaxValues();
  */


  datastruct1.value.resize(object1->numberOfNodes());
  //datastruct1.value = CalcSecondInvariant_Prism(object1, vel_u, vel_v, vel_w);
  datastruct1.value = CalcLengthSecondInvariant(object1, surface_node, SecondInvariant, th_length_sei);
  datastruct1.name = "Length Second Invariant";

  /*
  kvs::ValueArray<kvs::Real32> value2(&GradT.at(0), GradT.size() );
  object1->setValues( value2 );
  object1->updateMinMaxValues();
  */
  datastruct2.value.resize(object1->numberOfNodes());
  datastruct2.value = CalcLengthGradT(object1, surface_node, GradT);
  //datastruct2.value = CalcAbsGradT(object1, temperature);
  datastruct2.name = "Length GradT";


  histogramdata1.value.resize(object1->numberOfNodes());
  histogramdata2.value.resize(object1->numberOfNodes());
  std::copy(datastruct1.value.begin(), datastruct1.value.end(), histogramdata1.value.begin());
  std::copy(datastruct2.value.begin(), datastruct2.value.end(), histogramdata2.value.begin());
  histogramdata1.name = "SecondInvariant";
  histogramdata2.name = "GradT";

  kvs::ValueArray<kvs::UInt32> kvsConnection(&surface_polygon_connection.at(0), surface_polygon_connection.size() );
  /*
  object1->setNumberOfCells(surface_polygon_connection.size()/6);
  object1->setConnections(kvsConnection);
  */

  auto end_time_load = std::chrono::system_clock::now();

  kvs::TransferFunction tfunc_triangle=kvs::TransferFunction(kvs::RGBFormulae::Rainbow(256));
  std::vector<unsigned char> triangle_color1;
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_color1.push_back(255);
    triangle_color1.push_back(0);
    triangle_color1.push_back(0);
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleColor1(&triangle_color1.at(0), triangle_color1.size() );

  std::vector<unsigned char> triangle_color2;
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_color2.push_back(0);
    triangle_color2.push_back(0);
    triangle_color2.push_back(255);
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleColor2(&triangle_color2.at(0), triangle_color2.size() );

  std::vector<unsigned char> triangle_opacity1(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity1.at(i) = (unsigned char)(SecondInvariantColor(datastruct1.value.at(i)));
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity1(&triangle_opacity1.at(0), triangle_opacity1.size() );

  std::vector<unsigned char> triangle_opacity2(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    triangle_opacity2.at(i) = (unsigned char)(GradTColor(datastruct2.value.at(i)));
  }
  kvs::ValueArray<kvs::UInt8> kvsTriangleOpacity2(&triangle_opacity2.at(0), triangle_opacity2.size() );

  std::vector<unsigned int> triangle_connection;
  for(unsigned int i=0;i<surface_polygon_connection.size()/6;i++){
    for(unsigned int j=0;j<3;j++){
      triangle_connection.push_back(surface_polygon_connection.at(i*6+j));
    }
  }
  kvs::ValueArray<kvs::UInt32> kvsTriangleConnection(&triangle_connection.at(0), triangle_connection.size() );

  std::vector<float> normal;
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    float plength = LengthAandB(object1->coords()[i*3+0], object1->coords()[i*3+1], object1->coords()[i*3+2], 0,0,0);
    normal.push_back((1)*object1->coords()[i*3+0]/plength);
    normal.push_back((1)*object1->coords()[i*3+1]/plength);
    normal.push_back((1)*object1->coords()[i*3+2]/plength);
  }
  kvs::ValueArray<kvs::Real32> kvsNormal(&normal.at(0), normal.size() );

  //  create polygon1
  pobject1 = new kvs::PolygonObject();
  pobject1->setName( "Polygon1" );
  pobject1->setCoords( object1->coords() );
  pobject1->setColors( kvsTriangleColor1 );
  pobject1->setOpacities( kvsTriangleOpacity1 );
  pobject1->setNormals( kvsNormal );
  pobject1->setConnections( kvsTriangleConnection );
  pobject1->setPolygonType( kvs::PolygonObject::Triangle );
  pobject1->setColorType( kvs::PolygonObject::VertexColor );
  pobject1->setNormalType( kvs::PolygonObject::VertexNormal );

  pobject1->setMinMaxObjectCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );
  pobject1->setMinMaxExternalCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );

  //  create polygon2
  pobject2 = new kvs::PolygonObject();
  pobject2->setName( "Polygon2" );
  pobject2->setCoords( object1->coords() );
  pobject2->setColors( kvsTriangleColor2 );
  pobject2->setOpacities( kvsTriangleOpacity2 );
  pobject2->setNormals( kvsNormal );
  pobject2->setConnections( kvsTriangleConnection );
  pobject2->setPolygonType( kvs::PolygonObject::Triangle );
  pobject2->setColorType( kvs::PolygonObject::VertexColor );
  pobject2->setNormalType( kvs::PolygonObject::VertexNormal );

  pobject2->setMinMaxObjectCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );
  pobject2->setMinMaxExternalCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );

  // create background poygon
  pobject_background = new kvs::PolygonObject();
  pobject_background->setName( "PolygonBackground" );
  std::vector<float> bcoord(object1->numberOfNodes()*3);
  for(unsigned int i=0;i<object1->numberOfNodes()*3;i++){
    bcoord.at(i) = object1->coords()[i] * 19.8 / 20.0;
  }
  kvs::ValueArray<kvs::Real32> kvsBcoord(&bcoord.at(0), bcoord.size() );
  std::vector<unsigned char> bcolor(object1->numberOfNodes()*3, 255);
  /*
  for(unsigned int i=0;i<triangle_connection.size();i++){
    if(spot_flag == true){
      unsigned int index = triangle_connection.at(i);
      if(LengthAandB(object1->coords()[index*3+0], object1->coords()[index*3+1], object1->coords()[index*3+2], 22.35*cos(phi)*cos(theta), 22.35*cos(phi)*sin(theta), 22.35*sin(phi)) < r){
	bcolor.at(index*3+0)=0;
	bcolor.at(index*3+2)=0;
      }
    }
  }
  */
  kvs::ValueArray<kvs::UInt8> kvsBcolor(&bcolor.at(0), bcolor.size() );
  std::vector<unsigned char> bopacity(object1->numberOfNodes());
  for(unsigned int i=0;i<object1->numberOfNodes();i++){
    bopacity.at(i) = 255;
  }
  kvs::ValueArray<kvs::UInt8> kvsBopacity(&bopacity.at(0), bopacity.size() );
  pobject_background->setCoords( kvsBcoord );
  pobject_background->setColors( kvsBcolor );
  pobject_background->setOpacities( kvsBopacity );
  pobject_background->setNormals( kvsNormal );
  pobject_background->setConnections( kvsTriangleConnection );
  pobject_background->setPolygonType( kvs::PolygonObject::Triangle );
  pobject_background->setColorType( kvs::PolygonObject::VertexColor );
  pobject_background->setNormalType( kvs::PolygonObject::VertexNormal );
  pobject_background->setMinMaxObjectCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );
  pobject_background->setMinMaxExternalCoords( kvs::Vec3( -30, -30, -30 ), kvs::Vec3( 30, 30, 30 ) );

  kvs::StochasticPolygonRenderer *prenderer1 = new kvs::StochasticPolygonRenderer();
  kvs::StochasticPolygonRenderer *prenderer2 = new kvs::StochasticPolygonRenderer();
  kvs::StochasticPolygonRenderer *prenderer_back = new kvs::StochasticPolygonRenderer();

  prenderer1->setShader(kvs::Shader::BlinnPhong(0.2,0.8,0.1,1) );
  prenderer2->setShader(kvs::Shader::BlinnPhong(0.2,0.8,0.1,1) );

  //kvs::glut::Screen *screen = static_cast<kvs::glut::Screen*>(m_screen);

  //m_screen->setTitle( "Polygon Object" );
  m_screen->registerObject( pobject_background, prenderer_back );
  m_screen->registerObject( pobject1, prenderer1 );
  m_screen->registerObject( pobject2, prenderer2 );
  //m_screen->scene()->camera()->setPosition(kvs::Vector3f(4,-4,4),kvs::Vector3f(-1,1,-1));
  //m_screen->scene()->light()->setPosition(kvs::Vector3f(6,-6,6));
  compositor = new kvs::StochasticRenderingCompositor( m_screen->scene() );
  compositor->setRepetitionLevel( 20 );
  compositor->enableLODControl();
  compositor->enableRefinement();
  m_screen->setEvent( compositor );
  //screen->setBackgroundColor(kvs::RGBColor(0,0,0));
  m_screen->show();



  this->CreateHistogram( histogramdata1.value, histogramdata2.value, surface_node );
  auto end_time_total = std::chrono::system_clock::now();
  auto diff_time_total = end_time_total-start_time_total;
  auto diff_time_load = end_time_load-start_time_load;
  std::cout << "load time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time_load).count() << "msec" << std::endl;
  std::cout << "total time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time_total).count() << "msec" << std::endl;
}
