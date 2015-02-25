#include "loaducdfile.h"

#include <kvs/UnstructuredVolumeObject>
#include <kvs/DebugNew>
#include <kvs/AVSUcd>
#include <kvs/Message>
#include <kvs/Vector3>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <string>

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

  std::string LoadUcdFile::m_cell_type = "";
  size_t LoadUcdFile::m_nnodes = 0;
  size_t LoadUcdFile::m_ncells = 0;
  size_t                LoadUcdFile::m_veclen;
  kvs::AnyValueArray           LoadUcdFile::m_values;
  kvs::ValueArray<kvs::Real32> LoadUcdFile::m_coords;
  kvs::ValueArray<kvs::UInt32> LoadUcdFile::m_connections;

  long int coord_num;
  long int connection_num;
  long int tet_connection_num;
  long int prism_connection_num;

  LoadUcdFile::LoadUcdFile():
    m_writing_type( takami::LoadUcdFile::Ascii ),
    m_has_label( false ),
    m_label( "" ),
    m_has_min_value( false ),
    m_has_max_value( false ),
    m_min_value( 0.0 ),
    m_max_value( 0.0 )
{
}

  LoadUcdFile::LoadUcdFile(const std::string& filename ):
    m_writing_type( takami::LoadUcdFile::Ascii ),
    m_has_label( false ),
    m_label( "" ),
    m_has_min_value( false ),
    m_has_max_value( false ),
    m_min_value( 0.0 ),
    m_max_value( 0.0 )
{
  std::cout << "LoadUcdFile(const std::string& filename ) is not yet" << std::endl;
  /*
  if( this->read( filename ) ) {} //m_is_success = true; }
  else {} //m_is_success = false; }
  */
}

  LoadUcdFile::LoadUcdFile(const std::string& filename, char cell_type ):
    m_writing_type( takami::LoadUcdFile::Ascii ),
    m_has_label( false ),
    m_label( "" ),
    m_has_min_value( false ),
    m_has_max_value( false ),
    m_min_value( 0.0 ),
    m_max_value( 0.0 )
{
  if(cell_type == TETRA){
    m_cell_type = "tetrahedra";
  }else if(cell_type == PRISM){
    m_cell_type = "prism";
  }else{
    std::cout<<"loaducdfile.cpp's arg must be TETRA or PRISM" << std::endl;
    return;
  } 
  static bool is_read = this->read( filename );
  if( is_read==true ) {} //m_is_success = true; }
  else {std::cout<<"load miss"<<std::endl;} //m_is_success = false; }
}

  LoadUcdFile::~LoadUcdFile()
{
}


const std::string& LoadUcdFile::cellType( void ) const
{
    return( m_cell_type );
}

const bool LoadUcdFile::hasLabel( void ) const
{
    return( m_has_label );
}

const std::string& LoadUcdFile::label( void ) const
{
    return( m_label );
}

/*===========================================================================*/
/**
 *  @brief  Returns the vector length.
 *  @return vector length
 */
/*===========================================================================*/
const size_t LoadUcdFile::veclen( void ) const
{
    return( m_veclen );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of nodes.
 *  @return number of nodes
 */
/*===========================================================================*/
const size_t LoadUcdFile::nnodes( void ) const
{
    return( m_nnodes );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of cells.
 *  @return number of cells
 */
/*===========================================================================*/
const size_t LoadUcdFile::ncells( void ) const
{
    return( m_ncells );
}

const bool LoadUcdFile::hasMinValue( void ) const
{
    return( m_has_min_value );
}

const bool LoadUcdFile::hasMaxValue( void ) const
{
    return( m_has_max_value );
}

const double LoadUcdFile::minValue( void ) const
{
    return( m_min_value );
}

const double LoadUcdFile::maxValue( void ) const
{
    return( m_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the value array.
 *  @return value array
 */
/*===========================================================================*/
const kvs::AnyValueArray& LoadUcdFile::values( void ) const
{
    return( m_values );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate array.
 *  @return coordinate array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& LoadUcdFile::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the connection array.
 *  @return connection array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& LoadUcdFile::connections( void ) const
{
    return( m_connections );
}

/*===========================================================================*/
/**
 *  @brief  Sets a writing data type.
 *  @param  writing_type [in] writing data type
 */
/*===========================================================================*/
void LoadUcdFile::setWritingDataType( const WritingDataType writing_type )
{
    m_writing_type = writing_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a cell type.
 *  @param  cell_type [in] cell type
 */
/*===========================================================================*/
void LoadUcdFile::setCellType( const std::string& cell_type )
{
    m_cell_type = cell_type;
}

void LoadUcdFile::setLabel( const std::string& label )
{
    m_has_label = true;
    m_label = label;
}

/*===========================================================================*/
/**
 *  @brief  Sets a vector length.
 *  @param  veclen [in] vector length
 */
/*===========================================================================*/
void LoadUcdFile::setVeclen( const size_t veclen )
{
    m_veclen = veclen;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of nodes.
 *  @param  nnodes [in] number of nodes
 */
/*===========================================================================*/
void LoadUcdFile::setNNodes( const size_t nnodes )
{
    m_nnodes = nnodes;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of cells.
 *  @param  ncells [in] number of cells
 */
/*===========================================================================*/
void LoadUcdFile::setNCells( const size_t ncells )
{
    m_ncells = ncells;
}

void LoadUcdFile::setMinValue( const double min_value )
{
    m_has_min_value = true;
    m_min_value = min_value;
}

void LoadUcdFile::setMaxValue( const double max_value )
{
    m_has_max_value = true;
    m_max_value = max_value;
}

/*===========================================================================*/
/**
 *  @brief  Sets a value array.
 *  @param  values [in] value array
 */
/*===========================================================================*/
void LoadUcdFile::setValues( const kvs::AnyValueArray& values )
{
    m_values = values;
}

/*===========================================================================*/
/**
 *  @brief  Sets a coordinate array.
 *  @param  coords [in] coordinate array
 */
/*===========================================================================*/
void LoadUcdFile::setCoords( const kvs::ValueArray<kvs::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Sets a connection array.
 *  @param  connections [in] connection array
 */
/*===========================================================================*/
void LoadUcdFile::setConnections( const kvs::ValueArray<kvs::UInt32>& connections )
{
    m_connections = connections;
}

  bool LoadUcdFile::read( const std::string& filename )
{
  if(m_cell_type == "tetrahedra"){
    std::cout << "loading tetra file" << std::endl;
    read_tetra_file(filename);
    return(true);
  }else if(m_cell_type == "prism"){
    std::cout << "loading prism file" << std::endl;
    read_prism_file(filename);
    return(true);
  }else{
    std::cout << "Error, cell_type is not TETRA or PRISM." << std::endl;
    return(false);
  }
}

bool LoadUcdFile::write( const std::string& filename )
{
  std::cout << "writing cannot be used" << std::endl;
  return(false);
}

void LoadUcdFile::read_tetra_file( const std::string& filename )
{
  static kvs::ValueArray<kvs::Real32> coords;


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
    
  static kvs::ValueArray<kvs::UInt32> connections;
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
  long int value_num = coord_num*veclen;
  
  std::vector<float> tmp_value(value_num);
  
  fp.read(reinterpret_cast<char*>(&tmp_value.at(0)), value_num*sizeof(float));

  std::vector<float> tmp_value_exchange(value_num);
  for(unsigned int k=0;k<veclen;k++){
    for(unsigned int i=0;i<coord_num;i++){
      tmp_value_exchange.at(i*veclen+k) = tmp_value.at(k*coord_num+i);
    }
  }
  static kvs::ValueArray<kvs::Real32> values(&tmp_value_exchange.at(0), tmp_value_exchange.size());

  m_veclen = veclen;
  m_nnodes = coord_num;
  m_ncells = tet_connection_num;
  m_cell_type =  "tetrahedra" ;
  m_coords = coords;
  m_connections = connections;
  m_values = values;
  
  return;
}

void LoadUcdFile::read_prism_file( const std::string& filename )
{
  static kvs::ValueArray<kvs::Real32> coords;

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
  static kvs::ValueArray<kvs::UInt32> connections;
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
  long int value_num = coord_num*veclen;
  
  std::vector<float> tmp_value(value_num);
  
  fp.read(reinterpret_cast<char*>(&tmp_value.at(0)), value_num*sizeof(float));

  std::vector<float> tmp_value_exchange(value_num);
  for(unsigned int k=0;k<veclen;k++){
    for(unsigned int i=0;i<coord_num;i++){
      tmp_value_exchange.at(i*veclen+k) = tmp_value.at(k*coord_num+i);
    }
  }
  static kvs::ValueArray<kvs::Real32> values(&tmp_value_exchange.at(0), tmp_value_exchange.size());

  //static kvs::ValueArray<kvs::Real32> values(&tmp_value.at(0), tmp_value.size());
  
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
  

  m_veclen = veclen;
  m_nnodes = coord_num;
  m_ncells = prism_connection_num-count;
  m_cell_type = "prism";//::StringToCellType("prism");
  m_coords = coords;
  m_connections = connections;
  m_values = values;
  return;
}


}
