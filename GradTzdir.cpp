/*****************************************************************************/
/**
 *  @file   Grad.cpp
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
#include "GradTzdir.h"
#include "InverseDistanceWeighting.h"
#include "PrismCell.h"
#include <kvs/UnstructuredVolumeObject>
#include <kvs/PrismaticCell>


namespace
{

const kvs::UnstructuredVolumeObject* Cast( const kvs::ObjectBase* object )
{
    if ( !object )
    {
        kvsMessageError("Input object is NULL.");
        return NULL;
    }

    const kvs::VolumeObjectBase* volume = kvs::VolumeObjectBase::DownCast( object );
    if ( !volume )
    {
        kvsMessageError("Input object is not a volume data.");
        return NULL;
    }

    if ( volume->volumeType() != kvs::VolumeObjectBase::Unstructured )
    {
        kvsMessageError("Input object is not an unstructured volume object.");
        return NULL;
    }

    const kvs::UnstructuredVolumeObject* uvolume = kvs::UnstructuredVolumeObject::DownCast( volume );
    if ( uvolume->cellType() != kvs::UnstructuredVolumeObject::Prism )
    {
        kvsMessageError("Input object is not a prism mesh data.");
        return NULL;
    }

    return uvolume;
}

kvs::Vec3 GetCoord( const kvs::UnstructuredVolumeObject* volume, const size_t index )
{
    const kvs::Real32* coords = volume->coords().data();
    return kvs::Vec3( coords + index * 3 );
}

} // end of namespace


namespace local
{

GradTzdir::GradTzdir( const kvs::UnstructuredVolumeObject* volume ):
    kvs::FilterBase(),
    kvs::UnstructuredVolumeObject()
{
    this->exec( volume );
}

GradTzdir::SuperClass* GradTzdir::exec( const kvs::ObjectBase* object )
{
    const kvs::UnstructuredVolumeObject* volume = ::Cast( object );
    if ( !volume ) { return NULL; }

    if ( volume->veclen() == 3 )
    {
        this->vector_gradient( volume );
    }
    if( volume->veclen() == 1 )
      {
	this->scalar_gradient( volume );
      }
    return this;
}

void GradTzdir::scalar_gradient( const kvs::UnstructuredVolumeObject* volume )
{
    const size_t ncells = volume->numberOfCells();
    const size_t nnodes = volume->numberOfNodes();
    const kvs::UInt32* connections = volume->connections().data();

    local::InverseDistanceWeighting<kvs::Vec3> idw( nnodes );
    kvs::PrismaticCell cell( volume );
    const kvs::Vec3 center( 1.0f / 3.0f, 1.0f / 3.0f, 0.5f );
    for ( size_t i = 0; i < ncells; i++ )
    {
        cell.bindCell( i );
        cell.setLocalPoint( center );
        const kvs::Vec3 V = cell.gradient();

        const kvs::UInt32 id0 = *( connections++ );
        const kvs::UInt32 id1 = *( connections++ );
        const kvs::UInt32 id2 = *( connections++ );
        const kvs::UInt32 id3 = *( connections++ );
        const kvs::UInt32 id4 = *( connections++ );
        const kvs::UInt32 id5 = *( connections++ );

        const kvs::Vec3 x0 = ::GetCoord( volume, id0 );
        const kvs::Vec3 x1 = ::GetCoord( volume, id1 );
        const kvs::Vec3 x2 = ::GetCoord( volume, id2 );
        const kvs::Vec3 x3 = ::GetCoord( volume, id3 );
        const kvs::Vec3 x4 = ::GetCoord( volume, id4 );
        const kvs::Vec3 x5 = ::GetCoord( volume, id5 );
        const kvs::Vec3 xc = ( x0 + x1 + x2 + x3 + x4 + x5 ) / 6.0f;

        const kvs::Real32 d0 = ( x0 - xc ).length();
        const kvs::Real32 d1 = ( x1 - xc ).length();
        const kvs::Real32 d2 = ( x2 - xc ).length();
        const kvs::Real32 d3 = ( x3 - xc ).length();
        const kvs::Real32 d4 = ( x4 - xc ).length();
        const kvs::Real32 d5 = ( x5 - xc ).length();

        idw.insert( id0, V, d0 );
        idw.insert( id1, V, d1 );
        idw.insert( id2, V, d2 );
        idw.insert( id3, V, d3 );
        idw.insert( id4, V, d4 );
        idw.insert( id5, V, d5 );
    }

    SuperClass::shallowCopy( *volume );
    SuperClass::setVeclen( 3 );
    SuperClass::setValues( kvs::AnyValueArray( idw.serialize() ) );
    SuperClass::updateMinMaxValues();
}

void GradTzdir::vector_gradient( const kvs::UnstructuredVolumeObject* volume )
{
    const size_t ncells = volume->numberOfCells();
    const size_t nnodes = volume->numberOfNodes();
    const kvs::UInt32* connections = volume->connections().data();

    local::InverseDistanceWeighting<kvs::Mat3> idw( nnodes );
    local::PrismCell cell( volume );
    const kvs::Vec3 center( 1.0f / 3.0f, 1.0f / 3.0f, 0.5f );
    for ( size_t i = 0; i < ncells; i++ )
    {
        cell.bind( i );
        cell.setLocalPoint( center );
        const kvs::Mat3 T = cell.gradient();

        const kvs::UInt32 id0 = *( connections++ );
        const kvs::UInt32 id1 = *( connections++ );
        const kvs::UInt32 id2 = *( connections++ );
        const kvs::UInt32 id3 = *( connections++ );
        const kvs::UInt32 id4 = *( connections++ );
        const kvs::UInt32 id5 = *( connections++ );

        const kvs::Vec3 x0 = ::GetCoord( volume, id0 );
        const kvs::Vec3 x1 = ::GetCoord( volume, id1 );
        const kvs::Vec3 x2 = ::GetCoord( volume, id2 );
        const kvs::Vec3 x3 = ::GetCoord( volume, id3 );
        const kvs::Vec3 x4 = ::GetCoord( volume, id4 );
        const kvs::Vec3 x5 = ::GetCoord( volume, id5 );
        const kvs::Vec3 xc = ( x0 + x1 + x2 + x3 + x4 + x5 ) / 6.0f;

        const kvs::Real32 d0 = ( x0 - xc ).length();
        const kvs::Real32 d1 = ( x1 - xc ).length();
        const kvs::Real32 d2 = ( x2 - xc ).length();
        const kvs::Real32 d3 = ( x3 - xc ).length();
        const kvs::Real32 d4 = ( x4 - xc ).length();
        const kvs::Real32 d5 = ( x5 - xc ).length();

        idw.insert( id0, T, d0 );
        idw.insert( id1, T, d1 );
        idw.insert( id2, T, d2 );
        idw.insert( id3, T, d3 );
        idw.insert( id4, T, d4 );
        idw.insert( id5, T, d5 );
    }

    SuperClass::shallowCopy( *volume );
    SuperClass::setVeclen( 9 );
    SuperClass::setValues( kvs::AnyValueArray( idw.serialize() ) );
    SuperClass::updateMinMaxValues();
}

} // end of namespace
