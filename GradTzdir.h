/*****************************************************************************/
/**
 *  @file   GradTzdir.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#pragma once

#include <kvs/Module>
#include <kvs/FilterBase>
#include <kvs/UnstructuredVolumeObject>


namespace local
{

class GradTzdir : public kvs::FilterBase, public kvs::UnstructuredVolumeObject
{
    kvsModule( local::GradTzdir, Filter );
    kvsModuleBaseClass( kvs::FilterBase );
    kvsModuleSuperClass( kvs::UnstructuredVolumeObject );

public:

    GradTzdir( const kvs::UnstructuredVolumeObject* volume );
    SuperClass* exec( const kvs::ObjectBase* object );

private:

    void scalar_gradient( const kvs::UnstructuredVolumeObject* volume );
    void vector_gradient( const kvs::UnstructuredVolumeObject* volume );
};

} // end of namespace local
