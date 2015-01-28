#ifndef LOAD_UCD_H_INCLUDE
#define LOAD_UCD_H_INCLUDE

#include <kvs/ImporterBase>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/Vector3>
#include <vector>
#include <string>

namespace takami
{

/*==========================================================================*/
/**
 *  Unstructured volume object importer class.
 */
/*==========================================================================*/
class LoadUcd : public kvs::ImporterBase, public kvs::UnstructuredVolumeObject
{
public:

    LoadUcd();
    LoadUcd( const char* filename, char cell_type );
    LoadUcd( const char* filename, char cell_type, int NthValue );
    virtual ~LoadUcd();

private:

    int CheckFileLong(const char *filename);

    void LoadTetraObject_binary_long(const char *filename);
    void LoadPrismObject_binary_long(const char *filename);
    void LoadTetraCoordAndConnection( const char* filename, int NthValue);
    void LoadPrismCoordAndConnection( const char* filename, int NthValue);
    kvs::UnstructuredVolumeObject* exec( const kvs::FileFormatBase* file_format ){return(NULL);};
    //exec is dummy for importerbase

};


  std::vector<float> LoadUcdValue( const char* filename, int NthValue );
  std::string LoadUcdName( const char* filename, int NthValue );
  std::vector<float> CalcSecondInvariant(kvs::UnstructuredVolumeObject *object, std::vector<float> vel_u, std::vector<float> vel_v, std::vector<float> vel_w);
std::vector<float> CalcSecondInvariant_Prism(kvs::UnstructuredVolumeObject *object, std::vector<float> vel_u, std::vector<float> vel_v, std::vector<float> vel_w);
 std::vector<float> CalcSecondInvariant_Prism_pressure(kvs::UnstructuredVolumeObject *object, std::vector<float> p);
std::vector<float> CalcSecondInvariant_Prism_another(kvs::UnstructuredVolumeObject *object, std::vector<float> vel_u, std::vector<float> vel_v, std::vector<float> vel_w);
  float CalcLength(kvs::Real32 u, kvs::Real32 v, kvs::Real32 w, kvs::Real32 x, kvs::Real32 y, kvs::Real32 z );

  float CalcSecondInvariant_pointTopoint(float v_tensor00,float v_tensor01, float v_tensor02, float v_tensor10, float v_tensor11, float v_tensor12, float v_tensor20, float v_tensor21, float v_tensor22);
} // end of namespace takami



#endif
