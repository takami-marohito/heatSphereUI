#ifndef IMPORTER_H_INCLUDE
#define IMPORTER_H_INCLUDE

#include <kvs/ImporterBase>
#include <kvs/UnstructuredVolumeObject>
#include <string>
#include "loaducdfile.h"

namespace takami
{
  std::vector<float> LoadUcdValue( const char* filename, int NthValue );
  std::string LoadUcdName( const char* filename, int NthValue );

  class LoadUcd:public kvs::ImporterBase,public kvs::UnstructuredVolumeObject
    {
      
    public:
      LoadUcd();
      LoadUcd( const std::string& filename, char cell_type );
      LoadUcd( const std::string& filename, char cell_type, int NthValue );
      LoadUcd( char cell_type, int NthValue );
      virtual ~LoadUcd();
public:

    kvs::UnstructuredVolumeObject* exec( const kvs::FileFormatBase* file_format ){return(NULL);};

private:
    void LoadVolume(const std::string& filename, char cell_type);
    int CheckFileLong(const std::string& filename);
    void import( takami::LoadUcdFile* volume );
    void import( takami::LoadUcdFile* volume, int NthValue );
    takami::LoadUcdFile* m_volume;
    char m_cell_type;
};

} // end of namespace takami

#endif
