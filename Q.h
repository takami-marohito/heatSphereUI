#pragma once

#include <kvs/Module>
#include <kvs/FilterBase>
#include <kvs/UnstructuredVolumeObject>


namespace local
{

class Q : public kvs::FilterBase, public kvs::UnstructuredVolumeObject
{
    kvsModule( local::Q, Filter );
    kvsModuleBaseClass( kvs::FilterBase );
    kvsModuleSuperClass( kvs::UnstructuredVolumeObject );

public:

    Q( const kvs::UnstructuredVolumeObject* volume );
    SuperClass* exec( const kvs::ObjectBase* object );
};

} // end of namespace local
