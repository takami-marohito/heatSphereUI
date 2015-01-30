#pragma once

#include <kvs/Module>
#include <kvs/FilterBase>
#include <kvs/UnstructuredVolumeObject>


namespace local
{

class Qedge : public kvs::FilterBase, public kvs::UnstructuredVolumeObject
{
    kvsModule( local::Qedge, Filter );
    kvsModuleBaseClass( kvs::FilterBase );
    kvsModuleSuperClass( kvs::UnstructuredVolumeObject );

public:

    Qedge( const kvs::UnstructuredVolumeObject* volume );
    SuperClass* exec( const kvs::ObjectBase* object );
};

} // end of namespace local
