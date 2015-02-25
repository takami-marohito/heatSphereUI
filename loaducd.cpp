#include "loaducd.h"
#include <kvs/UnstructuredVolumeObject>
#include <kvs/Message>
#include <fstream>
#include <string>
#include <kvs/ValueArray>


#define TETRA 4
#define PRISM 6

namespace
{
const kvs::UnstructuredVolumeObject::CellType StringToCellType( const std::string& cell_type )
{
    if (      cell_type == "tetrahedra" ) { return kvs::UnstructuredVolumeObject::Tetrahedra; }
    else if ( cell_type == "quadratic tetrahedra" ) { return kvs::UnstructuredVolumeObject::QuadraticTetrahedra; }
    else if ( cell_type == "hexahedra"  ) { return kvs::UnstructuredVolumeObject::Hexahedra;  }
    else if ( cell_type == "quadratic hexahedra"  ) { return kvs::UnstructuredVolumeObject::QuadraticHexahedra;  }
    else if ( cell_type == "pyramid"  ) { return kvs::UnstructuredVolumeObject::Pyramid;  }
    else if ( cell_type == "point"  ) { return kvs::UnstructuredVolumeObject::Point;  }
    else if ( cell_type == "prism"  ) { return kvs::UnstructuredVolumeObject::Prism;  }
    else
    {
        kvsMessageError( "Unknown cell type '%s'.", cell_type.c_str() );
        return kvs::UnstructuredVolumeObject::UnknownCellType;
    }
}

} // end of namespace


namespace takami
{

  int LoadUcd::CheckFileLong(const std::string& filename)
  {
    std::ifstream fp( filename, std::ios::in | std::ios::binary );
    char* keyward = new char[100];
    memset( keyward, 0, 100 );
    fp.read( keyward, sizeof(char)*7 );
    fp.close();
    if(strcmp(keyward, "AVSUC64") == 0){
      return 0;
    }else if(strcmp(keyward, "AVS UCD") == 0){
      return 1;
    }
    return -1;
  }

  LoadUcd::LoadUcd(){};

  LoadUcd::LoadUcd( const std::string& filename, char cell_type )
  {
    if(this->CheckFileLong(filename)!=0){
      kvsMessageError( "LoadUcd cannot import short ucd file");
      return;
    }
    if(cell_type == TETRA || cell_type == PRISM){
      LoadVolume(filename, cell_type);
      this->import(m_volume);
      return;
    }
    kvsMessageError( "LoadUcd's 2nd arg is not TETRA or PRISM");
    return;
  }

  LoadUcd::LoadUcd( const std::string& filename, char cell_type, int NthValue )
  {
    if(this->CheckFileLong(filename)!=0){
      kvsMessageError( "LoadUcd cannot import short ucd file");
      return;
    }
    if(cell_type == TETRA || cell_type==PRISM){
      LoadVolume(filename, cell_type);
      this->import(m_volume, NthValue);
      return;
    }
    kvsMessageError( "LoadUcd's 2nd arg is not TETRA or PRISM");
    return;
  }

  LoadUcd::~LoadUcd()
  {
  }

  void LoadUcd::import(takami::LoadUcdFile* volume)
  {
    kvs::UnstructuredVolumeObject::setVeclen( volume->veclen() );
    kvs::UnstructuredVolumeObject::setNNodes( volume->nnodes() );
    kvs::UnstructuredVolumeObject::setNCells( volume->ncells() );
    kvs::UnstructuredVolumeObject::setCellType( ::StringToCellType(volume->cellType()) );
    kvs::UnstructuredVolumeObject::setCoords( volume->coords() );
    kvs::UnstructuredVolumeObject::setConnections( volume->connections() );
    kvs::UnstructuredVolumeObject::setValues( volume->values() );
    kvs::UnstructuredVolumeObject::updateMinMaxCoords();
    kvs::UnstructuredVolumeObject::updateMinMaxValues();
  }

  void LoadUcd::import(takami::LoadUcdFile* volume, int NthValue)
  {
    kvs::UnstructuredVolumeObject::setVeclen( 1 );
    kvs::UnstructuredVolumeObject::setNNodes( volume->nnodes() );
    kvs::UnstructuredVolumeObject::setNCells( volume->ncells() );
    kvs::UnstructuredVolumeObject::setCellType( ::StringToCellType(volume->cellType()) );
    kvs::UnstructuredVolumeObject::setCoords( volume->coords() );
    kvs::UnstructuredVolumeObject::setConnections( volume->connections() );
    kvs::UnstructuredVolumeObject::updateMinMaxCoords();

    kvs::ValueArray<kvs::Real32> NthValueArray( volume->nnodes() );
    float* p = NthValueArray.data();
    const float* buf = static_cast<const kvs::Real32*>(volume->values().data());
    for(int i=0;i<volume->nnodes();i++){
      p[i] = buf[i*volume->veclen()+NthValue];
    }
    kvs::UnstructuredVolumeObject::setValues( NthValueArray );
    kvs::UnstructuredVolumeObject::updateMinMaxValues();
  }

  void LoadUcd::LoadVolume(const std::string& filename, char cell_type)
  {
    m_volume = new takami::LoadUcdFile(filename, cell_type);
  }

  std::vector<float> LoadUcdValue( const char* filename, int NthValue )
  {
    kvs::ValueArray<kvs::Real32> coords;
    long int coord_num;
    long int connection_num;

    std::ifstream fp( filename, std::ios::in | std::ios::binary );
    //skip keyward
    fp.seekg(7*sizeof(char), std::ios_base::beg);
    //skip ucd's version
    fp.seekg(1*sizeof(float), std::ios_base::cur);
    //skip file title
    fp.seekg(70*sizeof(char), std::ios_base::cur);
    //skip n-th step
    fp.seekg(1*sizeof(int), std::ios_base::cur);
    //skip time
    fp.seekg(1*sizeof(float), std::ios_base::cur);

    //load coord_num = value_num
    fp.read( (char*)&coord_num, sizeof(long int) );
    //skip coord_describing_type
    fp.seekg(1*sizeof(int), std::ios_base::cur);

    //skip coord identifier
    fp.seekg(coord_num*sizeof(long int), std::ios_base::cur);

    //skip coord
    fp.seekg(coord_num*sizeof(float)*3, std::ios_base::cur);

    //load connection_num 
    fp.read( (char*)&connection_num, sizeof(long int) );

    //skip connection identifier
    fp.seekg(connection_num*sizeof(long int), std::ios_base::cur);

    //skip material array
    fp.seekg(connection_num*sizeof(int), std::ios_base::cur);

    //load element type
    std::vector<char> element_type(connection_num*sizeof(char));
    fp.read( reinterpret_cast<char*>(&element_type.at(0)), sizeof(char)*connection_num );

    //skip connection data
    long int total_connection_size=0;
    for(unsigned long int i=0;i<element_type.size();i++){
      if(element_type.at(i) == 4){
	total_connection_size = total_connection_size+4;
      }else if(element_type.at(i) == 6){
	total_connection_size = total_connection_size+6;
      }
    }
    fp.seekg(total_connection_size*sizeof(long int), std::ios_base::cur);
    
    kvs::ValueArray<kvs::UInt32> connections;

    //load veclen
    int veclen;
    fp.read( (char*)&veclen, sizeof(int) );

    //skip value describing type
    fp.seekg(sizeof(int), std::ios_base::cur);

    std::vector<char> value_name(16*veclen);
    std::vector<int> value_veclen(veclen);

    for(int i=0;i<veclen;i++){
      fp.read(reinterpret_cast<char*>(&value_name.at(16*i)), 16*sizeof(char));
      fp.seekg(sizeof(char)*16, std::ios_base::cur);
      fp.read(reinterpret_cast<char*>(&value_veclen.at(i)), sizeof(int));
      fp.seekg(sizeof(int), std::ios_base::cur);
      fp.seekg(sizeof(float), std::ios_base::cur);
    }

    long int value_num = coord_num;
    std::vector<float> values(value_num);

    char tmp_name[16]={0};
    memcpy(tmp_name, &value_name[NthValue*16], 15);
    std::cout << "loading " << tmp_name << std::endl;

    for(int i=0;i<NthValue;i++){
      fp.seekg(sizeof(float)*value_num, std::ios_base::cur);
    }
    
    fp.read(reinterpret_cast<char*>(&values.at(0)), value_num*sizeof(float));
    fp.close();
    return(values);
  }
  std::string LoadUcdName( const char* filename, int NthValue )
  {
    kvs::ValueArray<kvs::Real32> coords;
    long int coord_num;
    long int connection_num;

    std::ifstream fp( filename, std::ios::in | std::ios::binary );
    //skip keyward
    fp.seekg(7*sizeof(char), std::ios_base::beg);
    //skip ucd's version
    fp.seekg(1*sizeof(float), std::ios_base::cur);
    //skip file title
    fp.seekg(70*sizeof(char), std::ios_base::cur);
    //skip n-th step
    fp.seekg(1*sizeof(int), std::ios_base::cur);
    //skip time
    fp.seekg(1*sizeof(float), std::ios_base::cur);

    //load coord_num = value_num
    fp.read( (char*)&coord_num, sizeof(long int) );
    //skip coord_describing_type
    fp.seekg(1*sizeof(int), std::ios_base::cur);

    //skip coord identifier
    fp.seekg(coord_num*sizeof(long int), std::ios_base::cur);

    //skip coord
    fp.seekg(coord_num*sizeof(float)*3, std::ios_base::cur);

    //load connection_num 
    fp.read( (char*)&connection_num, sizeof(long int) );

    //skip connection identifier
    fp.seekg(connection_num*sizeof(long int), std::ios_base::cur);

    //skip material array
    fp.seekg(connection_num*sizeof(int), std::ios_base::cur);

    //load element type
    std::vector<char> element_type(connection_num*sizeof(char));
    fp.read( reinterpret_cast<char*>(&element_type.at(0)), sizeof(char)*connection_num );

    //skip connection data
    long int total_connection_size=0;
    for(unsigned long int i=0;i<element_type.size();i++){
      if(element_type.at(i) == 4){
	total_connection_size = total_connection_size+4;
      }else if(element_type.at(i) == 6){
	total_connection_size = total_connection_size+6;
      }
    }
    fp.seekg(total_connection_size*sizeof(long int), std::ios_base::cur);
    
    kvs::ValueArray<kvs::UInt32> connections;

    //load veclen
    int veclen;
    fp.read( (char*)&veclen, sizeof(int) );

    //skip value describing type
    fp.seekg(sizeof(int), std::ios_base::cur);

    std::vector<char> value_name(16*veclen);
    std::vector<int> value_veclen(veclen);

    for(int i=0;i<veclen;i++){
      fp.read(reinterpret_cast<char*>(&value_name.at(16*i)), 16*sizeof(char));
      fp.seekg(sizeof(char)*16, std::ios_base::cur);
      fp.read(reinterpret_cast<char*>(&value_veclen.at(i)), sizeof(int));
      fp.seekg(sizeof(int), std::ios_base::cur);
      fp.seekg(sizeof(float), std::ios_base::cur);
    }

    long int value_num = coord_num;
    std::vector<float> values(value_num);

    char tmp_name[16]={0};
    memcpy(tmp_name, &value_name[NthValue*16], 15);

    std::string return_name;
    return_name.append(tmp_name);

    fp.close();
    return(tmp_name);
  }

}
