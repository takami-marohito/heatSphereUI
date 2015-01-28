#include "loaducd.h"
#include <kvs/DebugNew>
#include <kvs/AVSUcd>
#include <kvs/Message>
#include <kvs/Vector3>
#include <fstream>
#include <math.h>
#include <algorithm>

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

  long int coord_num;
  long int connection_num;
  long int tet_connection_num;
  long int prism_connection_num;


  int LoadUcd::CheckFileLong(const char *filename){
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

LoadUcd::LoadUcd()
{
}

  LoadUcd::LoadUcd( const char* filename , char cell_type)
{
  if(cell_type == TETRA ){
    if(this->CheckFileLong(filename)==0){
      std::cout << "tetra long object" << std::endl;
      this->LoadTetraObject_binary_long(filename);
    }else if(this->CheckFileLong(filename)==1){
      std::cout << "tetra short object" << std::endl;
      kvsMessageError( "LoadUcd cannot import short ucd file");
      //this->LoadTetraObject_binary_short(filename);
    }
  }else if(cell_type == PRISM ){
    if(this->CheckFileLong(filename)==0){
      std::cout << "prism long object" << std::endl;
      this->LoadPrismObject_binary_long(filename);
    }else if(this->CheckFileLong(filename)==1){
      std::cout << "prism short object" << std::endl;
      kvsMessageError( "LoadUcd cannot import short ucd file");
      //this->LoadPrismObject_binary_short(filename);
    }
  }else{
    kvsMessageError( "This file is not binary ucd or file does not exist.");
  }
  return;
}

LoadUcd::LoadUcd( const char* filename , char cell_type, int NthValue)
{
  if(cell_type == TETRA ){
    if(this->CheckFileLong(filename)==0){
      std::cout << "tetra long object" << std::endl;
      this->LoadTetraCoordAndConnection(filename, NthValue);
    }else if(this->CheckFileLong(filename)==1){
      std::cout << "tetra short object" << std::endl;
      kvsMessageError( "LoadUcd cannot import short ucd file");
      //this->LoadTetraObject_binary_short(filename);
    }
  }else if(cell_type == PRISM ){
    if(this->CheckFileLong(filename)==0){
      std::cout << "prism long object" << std::endl;
      this->LoadPrismCoordAndConnection(filename, NthValue);
    }else if(this->CheckFileLong(filename)==1){
      std::cout << "prism short object" << std::endl;
      kvsMessageError( "LoadUcd cannot import short ucd file");
      //this->LoadPrismObject_binary_short(filename);
    }
  }else{
    kvsMessageError( "This file is not binary ucd or file does not exist.");
  }
  return;
}


LoadUcd::~LoadUcd()
{
}

  void LoadUcd::LoadTetraObject_binary_long(const char *filename){
    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::Real32> values;

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

    //the number of coord
    fp.read( (char*)&coord_num, sizeof(long int) );

    //skip coord_describing_type
    fp.seekg(1*sizeof(int), std::ios_base::cur);

    //skip coord identifier
    fp.seekg(coord_num*sizeof(long int), std::ios_base::cur);

    //load coord
    std::vector<float> coord_x(coord_num);
    std::vector<float> coord_y(coord_num); 
    std::vector<float> coord_z(coord_num); 

    fp.read( reinterpret_cast<char*>(&coord_x.at(0)), sizeof(float) * coord_num);
    fp.read( reinterpret_cast<char*>(&coord_y.at(0)), sizeof(float) * coord_num);
    fp.read( reinterpret_cast<char*>(&coord_z.at(0)), sizeof(float) * coord_num);

    coords.allocate( coord_num * 3 );
    for(long int i=0; i<coord_num; i++){
      coords.at(i*3+0) = coord_x[i];
      coords.at(i*3+1) = coord_y[i];
      coords.at(i*3+2) = coord_z[i];
    }

    //load connection_num 
    fp.read( (char*)&connection_num, sizeof(long int) );

    //skip connection identifier
    fp.seekg(connection_num*sizeof(long int), std::ios_base::cur);

    //skip material array
    fp.seekg(connection_num*sizeof(int), std::ios_base::cur);

    //load element type
    std::vector<char> element_type(connection_num*sizeof(char));
    fp.read( reinterpret_cast<char*>(&element_type.at(0)), sizeof(char)*connection_num );

    //calc element histogram
    std::vector<long int>element_histogram(14);

    for(long int i=0;i<connection_num;i++){
      element_histogram.at((int)element_type.at(i))++;
    }
    tet_connection_num = element_histogram.at(4);
    //prism_connection_num = element_histogram.at(6);

    std::vector<long int> tet_connection(tet_connection_num*4);

    long int count=0;
    for(long int i=0;i<connection_num;i++){
      if( (int)element_type.at(i) == 4 ){
	fp.read( reinterpret_cast<char*>(&tet_connection.at(count*4)), 4*sizeof(long int) );
	count++;
      }else if((int)element_type.at(i) == 6 ){
	fp.seekg(6*sizeof(long int), std::ios_base::cur);
      }else{
	std::cout << "this file includes not tetra and prism cell" << std::endl;
      }
    }
    
    kvs::ValueArray<kvs::UInt32> connections;
    connections.allocate(tet_connection_num*4);
    for(long int i=0;i<tet_connection_num*4;i++){
      connections.at(i) = (unsigned int)tet_connection.at(i)  ;
    }

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

    //value load
    long int value_num = coord_num;

    std::vector<float> vel_u(value_num);
    std::vector<float> vel_v(value_num);
    std::vector<float> vel_w(value_num);
    values.allocate( value_num );

    for(int i=0;i<5;i++){
      fp.seekg(sizeof(float)*value_num, std::ios_base::cur);
    }
    fp.read(reinterpret_cast<char*>(&vel_u.at(0)), value_num*sizeof(float));
    fp.read(reinterpret_cast<char*>(&vel_v.at(0)), value_num*sizeof(float));
    fp.read(reinterpret_cast<char*>(&vel_w.at(0)), value_num*sizeof(float));
    for(long int i=0;i<value_num;i++){
      values.at(i) = sqrt(vel_u[i]*vel_u[i]+vel_v[i]*vel_v[i]+vel_w[i]*vel_w[i]);
  }


    kvs::UnstructuredVolumeObject::setVeclen(1);
    kvs::UnstructuredVolumeObject::setNumberOfNodes( coord_num );
    kvs::UnstructuredVolumeObject::setNumberOfCells( tet_connection_num );
    kvs::UnstructuredVolumeObject::setCellType( ::StringToCellType( "tetrahedra" ) );
    kvs::UnstructuredVolumeObject::setCoords( coords );
    kvs::UnstructuredVolumeObject::setConnections( connections );
    kvs::UnstructuredVolumeObject::setValues( values );
    kvs::UnstructuredVolumeObject::updateMinMaxCoords();
    kvs::UnstructuredVolumeObject::updateMinMaxValues();

    return;
  }

  void LoadUcd::LoadPrismObject_binary_long(const char *filename){
    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::Real32> values;

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

    //the number of coord
    fp.read( (char*)&coord_num, sizeof(long int) );

    //skip coord_describing_type
    fp.seekg(1*sizeof(int), std::ios_base::cur);

    //skip coord identifier
    fp.seekg(coord_num*sizeof(long int), std::ios_base::cur);

    //load coord
    std::vector<float> coord_x(coord_num);
    std::vector<float> coord_y(coord_num); 
    std::vector<float> coord_z(coord_num); 

    fp.read( reinterpret_cast<char*>(&coord_x.at(0)), sizeof(float) * coord_num);
    fp.read( reinterpret_cast<char*>(&coord_y.at(0)), sizeof(float) * coord_num);
    fp.read( reinterpret_cast<char*>(&coord_z.at(0)), sizeof(float) * coord_num);

    coords.allocate( coord_num * 3 );
    for(long int i=0; i<coord_num; i++){
      coords.at(i*3+0) = coord_x[i];
      coords.at(i*3+1) = coord_y[i];
      coords.at(i*3+2) = coord_z[i];
    }

    //load connection_num 
    fp.read( (char*)&connection_num, sizeof(long int) );

    //skip connection identifier
    fp.seekg(connection_num*sizeof(long int), std::ios_base::cur);

    //skip material array
    fp.seekg(connection_num*sizeof(int), std::ios_base::cur);

    //load element type
    std::vector<char> element_type(connection_num*sizeof(char));
    fp.read( reinterpret_cast<char*>(&element_type.at(0)), sizeof(char)*connection_num );

    //calc element histogram
    std::vector<long int>element_histogram(14);

    for(long int i=0;i<connection_num;i++){
      element_histogram.at((int)element_type.at(i))++;
    }
    //tet_connection_num = element_histogram.at(4);
    prism_connection_num = element_histogram.at(6);

    std::vector<long int> prism_connection(prism_connection_num*6);

    long int count=0;
    for(long int i=0;i<connection_num;i++){
      if( (int)element_type.at(i) == 6 ){
	fp.read( reinterpret_cast<char*>(&prism_connection.at(count*6)), 6*sizeof(long int) );
	count++;
      }else if((int)element_type.at(i) == 4 ){
	fp.seekg(4*sizeof(long int), std::ios_base::cur);
      }else{
	std::cout << "this file includes not tetra and prism cell" << std::endl;
      }
    }
    kvs::ValueArray<kvs::UInt32> connections;
    connections.allocate(prism_connection_num*6);
    for(long int i=0;i<prism_connection_num*6;i++){
      connections.at(i) = (unsigned int)prism_connection.at(i)  ; 
    }
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

    //value load
    long int value_num = coord_num;

    std::vector<float> vel_u(value_num);
    std::vector<float> vel_v(value_num);
    std::vector<float> vel_w(value_num);
    values.allocate( value_num );

    for(int i=0;i<5;i++){
      fp.seekg(sizeof(float)*value_num, std::ios_base::cur);
    }
    fp.read(reinterpret_cast<char*>(&vel_u.at(0)), value_num*sizeof(float));
    fp.read(reinterpret_cast<char*>(&vel_v.at(0)), value_num*sizeof(float));
    fp.read(reinterpret_cast<char*>(&vel_w.at(0)), value_num*sizeof(float));
    for(long int i=0;i<value_num;i++){
      values.at(i) = sqrt(vel_u[i]*vel_u[i]+vel_v[i]*vel_v[i]+vel_w[i]*vel_w[i]);
    }
    
    std::vector<unsigned int> tmp_connection;
    if(6*(unsigned long int)prism_connection_num != connections.size()){
      kvsMessageError( "volume has %u cells and volume->connections().size() is %u.\nCutPrism's volume must be only prism.",prism_connection_num, connections.size() );
      return;
    }
    count = 0;
    for(unsigned int i=0;i<prism_connection_num;i++){
	if(coords.data()[3*connections.data()[i*6] ] < -300.0 || coords.data()[3*connections.data()[i*6] ] > 1000.0 ){
	  count++;
	}else{
	  for(unsigned int j=0;j<6;j++){
	    tmp_connection.push_back(connections.data()[i*6+j]);
	  }
	}
      }
    kvs::ValueArray<kvs::UInt32> connections_cut( tmp_connection.data(), tmp_connection.size() );
    
    kvs::UnstructuredVolumeObject::setVeclen(1);
    kvs::UnstructuredVolumeObject::setNumberOfNodes( coord_num );
    kvs::UnstructuredVolumeObject::setNumberOfCells( prism_connection_num-count );
    kvs::UnstructuredVolumeObject::setCellType( ::StringToCellType( "prism" ) );
    kvs::UnstructuredVolumeObject::setCoords( coords );
    kvs::UnstructuredVolumeObject::setConnections( connections_cut );
    kvs::UnstructuredVolumeObject::setValues( values );
    kvs::UnstructuredVolumeObject::updateMinMaxCoords();
    kvs::UnstructuredVolumeObject::updateMinMaxValues();
    return;
  }

void LoadUcd::LoadTetraCoordAndConnection( const char* filename, int NthValue){
    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::Real32> values;

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

    //the number of coord
    fp.read( (char*)&coord_num, sizeof(long int) );

    //skip coord_describing_type
    fp.seekg(1*sizeof(int), std::ios_base::cur);

    //skip coord identifier
    fp.seekg(coord_num*sizeof(long int), std::ios_base::cur);

    //load coord
    std::vector<float> coord_x(coord_num);
    std::vector<float> coord_y(coord_num); 
    std::vector<float> coord_z(coord_num); 

    fp.read( reinterpret_cast<char*>(&coord_x.at(0)), sizeof(float) * coord_num);
    fp.read( reinterpret_cast<char*>(&coord_y.at(0)), sizeof(float) * coord_num);
    fp.read( reinterpret_cast<char*>(&coord_z.at(0)), sizeof(float) * coord_num);

    coords.allocate( coord_num * 3 );
    for(long int i=0; i<coord_num; i++){
      coords.at(i*3+0) = coord_x[i];
      coords.at(i*3+1) = coord_y[i];
      coords.at(i*3+2) = coord_z[i];
    }

    //load connection_num 
    fp.read( (char*)&connection_num, sizeof(long int) );

    //skip connection identifier
    fp.seekg(connection_num*sizeof(long int), std::ios_base::cur);

    //skip material array
    fp.seekg(connection_num*sizeof(int), std::ios_base::cur);

    //load element type
    std::vector<char> element_type(connection_num*sizeof(char));
    fp.read( reinterpret_cast<char*>(&element_type.at(0)), sizeof(char)*connection_num );

    //calc element histogram
    std::vector<long int>element_histogram(14);

    for(long int i=0;i<connection_num;i++){
      element_histogram.at((int)element_type.at(i))++;
    }
    tet_connection_num = element_histogram.at(4);
    //prism_connection_num = element_histogram.at(6);

    std::vector<long int> tet_connection(tet_connection_num*4);

    long int count=0;
    for(long int i=0;i<connection_num;i++){
      if( (int)element_type.at(i) == 4 ){
	fp.read( reinterpret_cast<char*>(&tet_connection.at(count*4)), 4*sizeof(long int) );
	count++;
      }else if((int)element_type.at(i) == 6 ){
	fp.seekg(6*sizeof(long int), std::ios_base::cur);
      }else{
	std::cout << "this file includes not tetra and prism cell" << std::endl;
      }
    }
    
    kvs::ValueArray<kvs::UInt32> connections;
    connections.allocate(tet_connection_num*4);
    for(long int i=0;i<tet_connection_num*4;i++){
      connections.at(i) = (unsigned int)tet_connection.at(i)  ;
    }

    kvs::UnstructuredVolumeObject::setVeclen(1);
    kvs::UnstructuredVolumeObject::setNumberOfNodes( coord_num );
    kvs::UnstructuredVolumeObject::setNumberOfCells( tet_connection_num );
    kvs::UnstructuredVolumeObject::setCellType( ::StringToCellType( "tetrahedra" ) );
    kvs::UnstructuredVolumeObject::setCoords( coords );
    kvs::UnstructuredVolumeObject::setConnections( connections );
    kvs::UnstructuredVolumeObject::updateMinMaxCoords();

    if(NthValue == 0){
      return;
    }
    if(NthValue < 0){
      kvsMessageError("In LoadUcd(filename, cell_type, NthValue), NthValue start from 1.");
      return;
    }

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

    //value load
    long int value_num = coord_num;

    std::vector<float> vel_u(value_num);
    std::vector<float> vel_v(value_num);
    std::vector<float> vel_w(value_num);
    values.allocate( value_num );

    for(int i=0;i<NthValue-1;i++){
      fp.seekg(sizeof(float)*value_num, std::ios_base::cur);
    }
    fp.read(reinterpret_cast<char*>(&vel_u.at(0)), value_num*sizeof(float));
    fp.read(reinterpret_cast<char*>(&vel_v.at(0)), value_num*sizeof(float));
    fp.read(reinterpret_cast<char*>(&vel_w.at(0)), value_num*sizeof(float));
    for(long int i=0;i<value_num;i++){
      values.at(i) = sqrt(vel_u[i]*vel_u[i]+vel_v[i]*vel_v[i]+vel_w[i]*vel_w[i]);
    }

    kvs::UnstructuredVolumeObject::setValues( values );
    kvs::UnstructuredVolumeObject::updateMinMaxValues();
    return;
  }

void LoadUcd::LoadPrismCoordAndConnection(const char *filename, int NthValue){
    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::Real32> values;

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

    //the number of coord
    fp.read( (char*)&coord_num, sizeof(long int) );

    //skip coord_describing_type
    fp.seekg(1*sizeof(int), std::ios_base::cur);

    //skip coord identifier
    fp.seekg(coord_num*sizeof(long int), std::ios_base::cur);

    //load coord
    std::vector<float> coord_x(coord_num);
    std::vector<float> coord_y(coord_num); 
    std::vector<float> coord_z(coord_num); 

    fp.read( reinterpret_cast<char*>(&coord_x.at(0)), sizeof(float) * coord_num);
    fp.read( reinterpret_cast<char*>(&coord_y.at(0)), sizeof(float) * coord_num);
    fp.read( reinterpret_cast<char*>(&coord_z.at(0)), sizeof(float) * coord_num);

    coords.allocate( coord_num * 3 );
    for(long int i=0; i<coord_num; i++){
      coords.at(i*3+0) = coord_x[i];
      coords.at(i*3+1) = coord_y[i];
      coords.at(i*3+2) = coord_z[i];
    }

    //load connection_num 
    fp.read( (char*)&connection_num, sizeof(long int) );

    //skip connection identifier
    fp.seekg(connection_num*sizeof(long int), std::ios_base::cur);

    //skip material array
    fp.seekg(connection_num*sizeof(int), std::ios_base::cur);

    //load element type
    std::vector<char> element_type(connection_num*sizeof(char));
    fp.read( reinterpret_cast<char*>(&element_type.at(0)), sizeof(char)*connection_num );

    //calc element histogram
    std::vector<long int>element_histogram(14);

    for(long int i=0;i<connection_num;i++){
      element_histogram.at((int)element_type.at(i))++;
    }
    //tet_connection_num = element_histogram.at(4);
    prism_connection_num = element_histogram.at(6);

    std::vector<long int> prism_connection(prism_connection_num*6);

    long int count=0;
    for(long int i=0;i<connection_num;i++){
      if( (int)element_type.at(i) == 6 ){
	fp.read( reinterpret_cast<char*>(&prism_connection.at(count*6)), 6*sizeof(long int) );
	count++;
      }else if((int)element_type.at(i) == 4 ){
	fp.seekg(4*sizeof(long int), std::ios_base::cur);
      }else{
	std::cout << "this file includes not tetra and prism cell" << std::endl;
      }
    }
    kvs::ValueArray<kvs::UInt32> connections;
    connections.allocate(prism_connection_num*6);
    for(long int i=0;i<prism_connection_num*6;i++){
      connections.at(i) = (unsigned int)prism_connection.at(i)  ; 
    }

    
    std::vector<unsigned int> tmp_connection;
    if(6*(unsigned long int)prism_connection_num != connections.size()){
      kvsMessageError( "volume has %u cells and volume->connections().size() is %u.\nCutPrism's volume must be only prism.",prism_connection_num, connections.size() );
      return;
    }
    count = 0;
    for(unsigned int i=0;i<prism_connection_num;i++){
	if(coords.data()[3*connections.data()[i*6] ] < -300.0 || coords.data()[3*connections.data()[i*6] ] > 1000.0 ){
	  count++;
	}else{
	  for(unsigned int j=0;j<6;j++){
	    tmp_connection.push_back(connections.data()[i*6+j]);
	  }
	}
      }
    kvs::ValueArray<kvs::UInt32> connections_cut( tmp_connection.data(), tmp_connection.size() );
    
    kvs::UnstructuredVolumeObject::setVeclen(1);
    kvs::UnstructuredVolumeObject::setNumberOfNodes( coord_num );
    kvs::UnstructuredVolumeObject::setNumberOfCells( prism_connection_num-count );
    kvs::UnstructuredVolumeObject::setCellType( ::StringToCellType( "prism" ) );
    kvs::UnstructuredVolumeObject::setCoords( coords );
    kvs::UnstructuredVolumeObject::setConnections( connections_cut );
    kvs::UnstructuredVolumeObject::updateMinMaxCoords();

    if(NthValue == 0){
      return;
    }
    if(NthValue < 0){
      kvsMessageError("In LoadUcd(filename, cell_type, NthValue), NthValue start from 1.");
      return;
    }

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

    //value load
    long int value_num = coord_num;

    std::vector<float> vel_u(value_num);
    std::vector<float> vel_v(value_num);
    std::vector<float> vel_w(value_num);
    values.allocate( value_num );

    for(int i=0;i<NthValue-1;i++){
      fp.seekg(sizeof(float)*value_num, std::ios_base::cur);
    }
    fp.read(reinterpret_cast<char*>(&vel_u.at(0)), value_num*sizeof(float));
    fp.read(reinterpret_cast<char*>(&vel_v.at(0)), value_num*sizeof(float));
    fp.read(reinterpret_cast<char*>(&vel_w.at(0)), value_num*sizeof(float));
    for(long int i=0;i<value_num;i++){
      values.at(i) = sqrt(vel_u[i]*vel_u[i]+vel_v[i]*vel_v[i]+vel_w[i]*vel_w[i]);
    }

    kvs::UnstructuredVolumeObject::setValues( values );
    kvs::UnstructuredVolumeObject::updateMinMaxValues();
    return;
  }


std::vector<float> LoadUcdValue(const char *filename, int NthValue){
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
////end of LoadUcd class

std::string LoadUcdName(const char *filename, int NthValue){
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

std::vector<float> CalcSecondInvariant(kvs::UnstructuredVolumeObject *object, std::vector<float> vel_u, std::vector<float> vel_v, std::vector<float> vel_w){

  std::vector<float> SecInv(object->numberOfNodes(), 0);

  std::vector< std::vector<unsigned int> > point_connection;
  point_connection.resize(object->numberOfNodes());
  unsigned int index;

  for(unsigned int i=0;i<object->numberOfCells();i++){
    if(i%(object->numberOfCells()/10) == 1){
      printf("%f\n",(float)(i)/(float)object->numberOfCells());
    }
    for(unsigned int j=0;j<4;j++){
      index = object->connections()[i*4+j];
      for(unsigned int k=0;k<4;k++){
	if(j==k){
	}else{
	  unsigned int count=0;
	  for(unsigned int l=0;l<point_connection[index].size();l++){
	    if(object->connections()[i*4+k] != point_connection[index].at(l) ){
	      count++;
	    }
	  }
	  if(count == point_connection[index].size()){
	    point_connection[index].push_back(object->connections()[i*4+k]);
	  }
	}
      }
    }
  }
  //point_connection[i] has numbers of i-th vertex's line connection.


  for(unsigned int i=0;i<object->numberOfNodes();i++){
    if(i%(object->numberOfNodes()/10) == 1){
      printf("%f\n",(float)(i)/(float)object->numberOfNodes());
    }
    float sum_length = 0;
    float sum_value=0;

    for(unsigned int j=0;j<point_connection[i].size();j++){
      float length = CalcLength(object->coords()[i*3+0], object->coords()[i*3+1], object->coords()[i*3+2], object->coords()[point_connection[i].at(j)*3+0], object->coords()[point_connection[i].at(j)*3+1], object->coords()[point_connection[i].at(j)*3+2] );
      sum_length += length;

      unsigned int index1=i;
      unsigned int index2=point_connection[i].at(j);
      
      float v_tensor00,v_tensor01, v_tensor02, v_tensor10, v_tensor11, v_tensor12, v_tensor20, v_tensor21, v_tensor22;

      kvs::Real32 x = object->coords()[index1*3+0];
      kvs::Real32 y = object->coords()[index1*3+1];
      kvs::Real32 z = object->coords()[index1*3+2];

      kvs::Real32 u = object->coords()[index2*3+0];
      kvs::Real32 v = object->coords()[index2*3+1];
      kvs::Real32 w = object->coords()[index2*3+2];

      v_tensor00 = (vel_u.at(index1)-vel_u.at(index2))/(x-u);
      v_tensor10 = (vel_u.at(index1)-vel_u.at(index2))/(y-v);
      v_tensor20 = (vel_u.at(index1)-vel_u.at(index2))/(z-w);
      v_tensor01 = (vel_v.at(index1)-vel_v.at(index2))/(x-u);
      v_tensor11 = (vel_v.at(index1)-vel_v.at(index2))/(y-v);
      v_tensor21 = (vel_v.at(index1)-vel_v.at(index2))/(z-w);
      v_tensor02 = (vel_w.at(index1)-vel_w.at(index2))/(x-u);
      v_tensor12 = (vel_w.at(index1)-vel_w.at(index2))/(y-v);
      v_tensor22 = (vel_w.at(index1)-vel_w.at(index2))/(z-w);

      if(100 < v_tensor00 || 100 < v_tensor01 || 100 < v_tensor02 || 100 < v_tensor10 || 100 < v_tensor20 || 100 < v_tensor11 || 100 < v_tensor12 || 100 < v_tensor22 || 100 < v_tensor21){
      }else if(-100 > v_tensor00 || -100 > v_tensor01 || -100 > v_tensor02 || -100 > v_tensor10 || -100 > v_tensor20 || -100 > v_tensor11 || -100 > v_tensor12 || -100 > v_tensor22 || -100 > v_tensor21){
      }else{

      sum_value += length*CalcSecondInvariant_pointTopoint(v_tensor00,v_tensor01, v_tensor02, v_tensor10, v_tensor11, v_tensor12, v_tensor20, v_tensor21, v_tensor22);
      }
    }
    if(point_connection[i].size() == 0){
      SecInv.at(i) = -1;
    }else if(sum_length == 0){
      SecInv.at(i) = 0;
    }else{
      //std::cout << tmp  << std::endl;
      //SecInv.at(i) = sum_value;
      SecInv.at(i) = sum_value/sum_length;
      //SecInv.at(i) = 50;
    }
  }
  return(SecInv);
}

float CalcLength(kvs::Real32 u, kvs::Real32 v, kvs::Real32 w, kvs::Real32 x, kvs::Real32 y, kvs::Real32 z ){
  return (sqrt( (u-x)*(u-x)+(v-y)*(v-y)+(w-z)*(w-z) ));
}

float CalcSecondInvariant_pointTopoint(float v_tensor00,float v_tensor01, float v_tensor02, float v_tensor10, float v_tensor11, float v_tensor12, float v_tensor20, float v_tensor21, float v_tensor22){
  return(  v_tensor01*v_tensor10 + v_tensor12*v_tensor21 + v_tensor20*v_tensor02 - v_tensor00*v_tensor11 - v_tensor11*v_tensor22 - v_tensor22*v_tensor00 );
}

std::vector<float> CalcSecondInvariant_Prism(kvs::UnstructuredVolumeObject *object, std::vector<float> vel_u, std::vector<float> vel_v, std::vector<float> vel_w){
  std::cout <<object->numberOfCells()<<std::endl;
  std::vector<float> SecInv(object->numberOfNodes(), 0);

  std::vector< std::vector<unsigned int> > point_connection;
  point_connection.resize(object->numberOfNodes());
  unsigned int index;

  for(unsigned int i=0;i<object->numberOfCells();i++){
    if(i%(object->numberOfCells()/10) == 1){
      printf("%f\n",(float)(i)/(float)object->numberOfCells());
    }
    for(unsigned int j=0;j<6;j++){
      index = object->connections()[i*6+j];
      if(j<=3){
	for(unsigned int k=0;k<3;k++){
	  if(j==k){
	  }else{
	    unsigned int count=0;
	    for(unsigned int l=0;l<point_connection[index].size();l++){
	      if(object->connections()[i*6+k] != point_connection[index].at(l) ){
		count++;
	      }
	    }
	    if(count == point_connection[index].size()){
	      point_connection[index].push_back(object->connections()[i*6+k]);
	    }
	  }
	}
	unsigned int count=0;
	for(unsigned int l=0;l<point_connection[index].size();l++){
	  if(object->connections()[index+3] != point_connection[index].at(l) ){
	    count++;
	  }
	}
	if(count == point_connection[index].size()){
	  point_connection[index].push_back(object->connections()[index+3]);
	}
      }
      if(j>=4){
	for(unsigned int k=3;k<6;k++){
	  if(j==k){
	  }else{
	    unsigned int count=0;
	    for(unsigned int l=0;l<point_connection[index].size();l++){
	      if(object->connections()[i*6+k] != point_connection[index].at(l) ){
		count++;
	      }
	    }
	    if(count == point_connection[index].size()){
	      point_connection[index].push_back(object->connections()[i*6+k]);
	    }
	  }
	}
	unsigned int count=0;
	for(unsigned int l=0;l<point_connection[index].size();l++){
	  if(object->connections()[index-3] != point_connection[index].at(l) ){
	    count++;
	  }
	}
	if(count == point_connection[index].size()){
	  point_connection[index].push_back(object->connections()[index-3]);
	}
      }
    }
  }
  //point_connection[i] has numbers of i-th vertex's line connection.

  int c_count = 0;
  for(unsigned int i=0;i<object->numberOfNodes();i++){
    if(i%(object->numberOfNodes()/10) == 1){
      printf("%f\n",(float)(i)/(float)object->numberOfNodes());
    }
    float sum_length = 0;
    float sum_value=0;

    for(unsigned int j=0;j<point_connection[i].size();j++){
      float length = CalcLength(object->coords()[i*3+0], object->coords()[i*3+1], object->coords()[i*3+2], object->coords()[point_connection[i].at(j)*3+0], object->coords()[point_connection[i].at(j)*3+1], object->coords()[point_connection[i].at(j)*3+2] );

      unsigned int index1=i;
      unsigned int index2=point_connection[i].at(j);
      
      float v_tensor00,v_tensor01, v_tensor02, v_tensor10, v_tensor11, v_tensor12, v_tensor20, v_tensor21, v_tensor22;

      kvs::Real32 x = object->coords()[index1*3+0];
      kvs::Real32 y = object->coords()[index1*3+1];
      kvs::Real32 z = object->coords()[index1*3+2];

      kvs::Real32 u = object->coords()[index2*3+0];
      kvs::Real32 v = object->coords()[index2*3+1];
      kvs::Real32 w = object->coords()[index2*3+2];

      v_tensor00 = (vel_u.at(index1)-vel_u.at(index2))/(x-u);
      v_tensor10 = (vel_u.at(index1)-vel_u.at(index2))/(y-v);
      v_tensor20 = (vel_u.at(index1)-vel_u.at(index2))/(z-w);
      v_tensor01 = (vel_v.at(index1)-vel_v.at(index2))/(x-u);
      v_tensor11 = (vel_v.at(index1)-vel_v.at(index2))/(y-v);
      v_tensor21 = (vel_v.at(index1)-vel_v.at(index2))/(z-w);
      v_tensor02 = (vel_w.at(index1)-vel_w.at(index2))/(x-u);
      v_tensor12 = (vel_w.at(index1)-vel_w.at(index2))/(y-v);
      v_tensor22 = (vel_w.at(index1)-vel_w.at(index2))/(z-w);

      /*
      if(1000 < v_tensor00 || 1000 < v_tensor01 || 1000 < v_tensor02 || 1000 < v_tensor10 || 1000 < v_tensor20 || 1000 < v_tensor11 || 1000 < v_tensor12 || 1000 < v_tensor22 || 1000 < v_tensor21){
      }else if(-1000 > v_tensor00 || -1000 > v_tensor01 || -1000 > v_tensor02 || -1000 > v_tensor10 || -1000 > v_tensor20 || -1000 > v_tensor11 || -1000 > v_tensor12 || -1000 > v_tensor22 || -1000 > v_tensor21){
      }else 
      */
      if(NAN == v_tensor00 || NAN == v_tensor01 || NAN == v_tensor02 || NAN == v_tensor10 || NAN == v_tensor20 || NAN == v_tensor11 || NAN == v_tensor12 || NAN == v_tensor22 || NAN == v_tensor21){
      }else if(-NAN == v_tensor00 || -NAN == v_tensor01 || -NAN == v_tensor02 || -NAN == v_tensor10 || -NAN == v_tensor20 || -NAN == v_tensor11 || -NAN == v_tensor12 || -NAN == v_tensor22 || -NAN == v_tensor21){
      }else{
      
      sum_value += 1/length*CalcSecondInvariant_pointTopoint(v_tensor00,v_tensor01, v_tensor02, v_tensor10, v_tensor11, v_tensor12, v_tensor20, v_tensor21, v_tensor22);
      sum_length += 1/length;
      }
      
    }
    if(point_connection[i].size() == 0){
      SecInv.at(i) = 0;
    }else if(sum_length == 0){
      SecInv.at(i) = 0;
    }else{
      //std::cout << tmp  << std::endl;
      //SecInv.at(i) = sum_value;
      SecInv.at(i) = sum_value/sum_length;
      if(SecInv.at(i) < 0){
	SecInv.at(i) = 0;
      }
      if(SecInv.at(i) > 0.000001){
	SecInv.at(i)=0.000001;
	c_count++;
      }
      SecInv.at(i) = SecInv.at(i)*1000000 ;
      //SecInv.at(i) = 50;
      if(isnan(SecInv.at(i)) ){
	SecInv.at(i) = 0;
      }
    }
  }
  std::cout << "max SecondInvariant is " << *std::max_element( SecInv.begin(), SecInv.end() )  << "   min is " << *std::min_element( SecInv.begin(), SecInv.end() ) << std::endl;
  std::cout << "number of points over the limit of SecInv is " << c_count << std::endl;
  std::cout << "number of total points is " << object->numberOfNodes() << std::endl;
  return(SecInv);
}


std::vector<float> CalcSecondInvariant_Prism_another(kvs::UnstructuredVolumeObject *object, std::vector<float> vel_u, std::vector<float> vel_v, std::vector<float> vel_w){
  std::cout <<object->numberOfCells()<<std::endl;
  std::vector<float> SecInv(object->numberOfNodes(), 0);
  std::vector<float> sum_length(object->numberOfNodes(), 0);
  std::vector<float> sum_v_tensor(object->numberOfNodes()*9,0);

  int dx_cut_count = 0;

  for(unsigned int i=0;i<object->numberOfCells();i++){
    for(unsigned int j=0;j<6;j++){
      for(unsigned int k=0;k<6;k++){
	if(j!=k){
	  float length = CalcLength(object->coords()[object->connections()[i*6+j]*3+0], object->coords()[object->connections()[i*6+j]*3+1], object->coords()[object->connections()[i*6+j]*3+2], object->coords()[object->connections()[i*6+k]*3+0], object->coords()[object->connections()[i*6+k]*3+1], object->coords()[object->connections()[i*6+k]*3+2] );

	  unsigned int index1 = object->connections()[i*6+j];
	  unsigned int index2 = object->connections()[i*6+k];

	  float v_tensor00,v_tensor01, v_tensor02, v_tensor10, v_tensor11, v_tensor12, v_tensor20, v_tensor21, v_tensor22;

	  kvs::Real32 x = object->coords()[index1*3+0];
	  kvs::Real32 y = object->coords()[index1*3+1];
	  kvs::Real32 z = object->coords()[index1*3+2];

	  kvs::Real32 u = object->coords()[index2*3+0];
	  kvs::Real32 v = object->coords()[index2*3+1];
	  kvs::Real32 w = object->coords()[index2*3+2];

	  v_tensor00 = (vel_u.at(index1)-vel_u.at(index2))/(x-u);
	  v_tensor10 = (vel_u.at(index1)-vel_u.at(index2))/(y-v);
	  v_tensor20 = (vel_u.at(index1)-vel_u.at(index2))/(z-w);
	  v_tensor01 = (vel_v.at(index1)-vel_v.at(index2))/(x-u);
	  v_tensor11 = (vel_v.at(index1)-vel_v.at(index2))/(y-v);
	  v_tensor21 = (vel_v.at(index1)-vel_v.at(index2))/(z-w);
	  v_tensor02 = (vel_w.at(index1)-vel_w.at(index2))/(x-u);
	  v_tensor12 = (vel_w.at(index1)-vel_w.at(index2))/(y-v);
	  v_tensor22 = (vel_w.at(index1)-vel_w.at(index2))/(z-w);

	  if(x-u < 0.000001 || y-v < 0.000001 || z-w < 0.000001){
	    dx_cut_count++;
	  }else if(NAN == v_tensor00 || NAN == v_tensor01 || NAN == v_tensor02 || NAN == v_tensor10 || NAN == v_tensor20 || NAN == v_tensor11 || NAN == v_tensor12 || NAN == v_tensor22 || NAN == v_tensor21){
	  }else if(-NAN == v_tensor00 || -NAN == v_tensor01 || -NAN == v_tensor02 || -NAN == v_tensor10 || -NAN == v_tensor20 || -NAN == v_tensor11 || -NAN == v_tensor12 || -NAN == v_tensor22 || -NAN == v_tensor21){
	  }else{
	    sum_v_tensor.at(index1*9+0) += 1.0/length*v_tensor00;
	    sum_v_tensor.at(index1*9+3) += 1.0/length*v_tensor10;
	    sum_v_tensor.at(index1*9+6) += 1.0/length*v_tensor20;
	    sum_v_tensor.at(index1*9+1) += 1.0/length*v_tensor01;
	    sum_v_tensor.at(index1*9+4) += 1.0/length*v_tensor11;
	    sum_v_tensor.at(index1*9+7) += 1.0/length*v_tensor21;
	    sum_v_tensor.at(index1*9+2) += 1.0/length*v_tensor02;
	    sum_v_tensor.at(index1*9+5) += 1.0/length*v_tensor12;
	    sum_v_tensor.at(index1*9+8) += 1.0/length*v_tensor22;
	    sum_length.at(index1) += 1.0/length;
	  }
	}
      }
    }
  }

  int c_count = 0;

  for(unsigned int i=0;i<object->numberOfNodes();i++){
    if(sum_length.at(i)==0){
      SecInv.at(i)=0;
    }else{
      SecInv.at(i) = CalcSecondInvariant_pointTopoint(sum_v_tensor.at(i*9+0)/sum_length.at(i),sum_v_tensor.at(i*9+1)/sum_length.at(i),sum_v_tensor.at(i*9+2)/sum_length.at(i),sum_v_tensor.at(i*9+3)/sum_length.at(i),sum_v_tensor.at(i*9+4)/sum_length.at(i),sum_v_tensor.at(i*9+5)/sum_length.at(i),sum_v_tensor.at(i*9+6)/sum_length.at(i),sum_v_tensor.at(i*9+7)/sum_length.at(i),sum_v_tensor.at(i*9+8)/sum_length.at(i));
    }
    if(SecInv.at(i) < 0){
      SecInv.at(i) = 0;
    }
    if(SecInv.at(i) > 200){
      SecInv.at(i)=200;
      c_count++;
    }
    SecInv.at(i) = SecInv.at(i)/200 ;

    if(isnan(SecInv.at(i)) ){
      SecInv.at(i) = 0;
    }
  }

  std::cout << "max SecondInvariant is " << *std::max_element( SecInv.begin(), SecInv.end() )  << "   min is " << *std::min_element( SecInv.begin(), SecInv.end() ) << std::endl;
  std::cout << "number of points over the limit of SecInv is " << c_count << std::endl;
  std::cout << "number of total points is " << object->numberOfNodes() << std::endl;
  printf("cut_count %d\n",dx_cut_count);
  return(SecInv);
}

std::vector<float> CalcSecondInvariant_Prism_pressure(kvs::UnstructuredVolumeObject *object, std::vector<float> p){
  std::cout <<object->numberOfCells()<<std::endl;
  std::vector<float> SecInv(object->numberOfNodes(), 0);
  std::vector<float> sum_length(object->numberOfNodes(), 0);
  std::vector<float> sum_v_tensor(object->numberOfNodes()*9,0);

  int dx_cut_count = 0;

  for(unsigned int i=0;i<object->numberOfCells();i++){
    for(unsigned int j=0;j<6;j++){
      for(unsigned int k=0;k<6;k++){
	if(j!=k){
	  float length = CalcLength(object->coords()[object->connections()[i*6+j]*3+0], object->coords()[object->connections()[i*6+j]*3+1], object->coords()[object->connections()[i*6+j]*3+2], object->coords()[object->connections()[i*6+k]*3+0], object->coords()[object->connections()[i*6+k]*3+1], object->coords()[object->connections()[i*6+k]*3+2] );

	  unsigned int index1 = object->connections()[i*6+j];
	  unsigned int index2 = object->connections()[i*6+k];

	  kvs::Real32 x = object->coords()[index1*3+0];
	  kvs::Real32 y = object->coords()[index1*3+1];
	  kvs::Real32 z = object->coords()[index1*3+2];

	  kvs::Real32 u = object->coords()[index2*3+0];
	  kvs::Real32 v = object->coords()[index2*3+1];
	  kvs::Real32 w = object->coords()[index2*3+2];

	  kvs::Real32 dx2 = (x-u)*(x-u);
	  kvs::Real32 dy2 = (y-v)*(y-v);
	  kvs::Real32 dz2 = (z-w)*(z-w);
	  kvs::Real32 dp = p[index1]-p[index2];

	  if(x-u < 0.000001 || y-v < 0.000001 || z-w < 0.000001){
	    dx_cut_count++;
	  }else{
	    SecInv.at(index1) += 1.0/length*dp/dx2+length*dp/dy2+length*dp/dz2;
	    sum_length.at(index1) += 1.0/length;
	  }
	}
      }
    }
  }

  int c_count = 0;

  for(unsigned int i=0;i<object->numberOfNodes();i++){
    if(sum_length.at(i)==0){
      SecInv.at(i)=0;
    }else{
      SecInv.at(i) = SecInv.at(i) / sum_length.at(i);
    }
    if(i%1000000==0){
      printf("a %f\n",SecInv.at(i));
    }
    if(SecInv.at(i) < 0){
      SecInv.at(i) = 0;
    }
    if(SecInv.at(i) > 30000){
      SecInv.at(i)=30000;
      c_count++;
    }
    SecInv.at(i) = SecInv.at(i)/30000 ;

    if(isnan(SecInv.at(i)) ){
      SecInv.at(i) = 0;
    }
  }

  std::cout << "max SecondInvariant is " << *std::max_element( SecInv.begin(), SecInv.end() )  << "   min is " << *std::min_element( SecInv.begin(), SecInv.end() ) << std::endl;
  std::cout << "number of points over the limit of SecInv is " << c_count << std::endl;
  std::cout << "number of total points is " << object->numberOfNodes() << std::endl;
  printf("cut_count %d\n",dx_cut_count);
  return(SecInv);
}

} // end of namespace takami
