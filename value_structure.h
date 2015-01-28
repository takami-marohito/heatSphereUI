#ifndef VALUE_STRUCTURE_H
#define VALUE_STRUCTURE_H

#include <vector>
#include <string>

#include <QColor>

struct ValueStruct{
  std::vector<float> value;
  std::string name;
public:
  void setData(const std::vector<float> &inputvalue, const std::string &inputname)
{
  value.resize(inputvalue.size());
  std::copy(inputvalue.begin(), inputvalue.end(), value.begin());
  name = inputname;
  return;
};
};


struct TransferFunction2D{
  std::vector<QColor> cmap;
  std::string name;
  std::vector<float> omap;
public:
  TransferFunction2D( const std::vector<QColor> &inputcmap, std::vector<float> &inputomap, const std::string &inputname)
  {
  cmap.resize(inputcmap.size());
  std::copy(inputcmap.begin(), inputcmap.end(), cmap.begin());
  omap.resize(inputomap.size());
  std::copy(inputomap.begin(), inputomap.end(), omap.begin());
  name = inputname;
  return;
} ;
  TransferFunction2D(){};


  void setData(const std::vector<QColor> &inputcmap, std::vector<float> &inputomap, const std::string &inputname)
{
  cmap.resize(inputcmap.size());
  std::copy(inputcmap.begin(), inputcmap.end(), cmap.begin());
  omap.resize(inputomap.size());
  std::copy(inputomap.begin(), inputomap.end(), omap.begin());
  name = inputname;
  return;
} ;
  void setOmap( std::vector<float> &inputomap )
{
  omap.resize(inputomap.size());
  std::copy(inputomap.begin(), inputomap.end(), omap.begin());
  return;
} ;
  void setCmap(const std::vector<QColor> &inputcmap)
{
  cmap.resize(inputcmap.size());
  std::copy(inputcmap.begin(), inputcmap.end(), cmap.begin());
  return;
} ;
  void setName(const std::string &inputname)
{
  name = inputname;
  return;
} ;
void resize(int size, float init)
{
  omap.resize(size, init);
  cmap.resize(size, init);
  return;
} ;
void resize(int size)
{
  omap.resize(size);
  cmap.resize(size);
  return;
} ;
void clear()
{
  omap.clear();
  cmap.clear();
  name.resize(0);
};
void setOmap_at( int x, int y, float value )
{
  omap.at(x+y*256)=value;
  return;
} ;
void setCmap_at( int x, int y, QColor value )
{
  cmap.at(x+y*256)=value;
  return;
} ;
};


#endif
