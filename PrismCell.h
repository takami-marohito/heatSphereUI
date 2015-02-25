/*****************************************************************************/
/**
 *  @file   PrismCell.h
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

#include <kvs/Vector3>
#include <kvs/Matrix33>
#include <kvs/UnstructuredVolumeObject>


namespace local
{

class PrismCell
{
    kvs::Vec3 m_coords[6]; ///< coordinate values
    kvs::Vec3 m_values[6]; ///< vector values
    kvs::Real32 m_N[6]; ///< interpolation functions
    kvs::Real32 m_dNdp[6]; ///< differencial interpolation functions
    kvs::Real32 m_dNdq[6]; ///< differencial interpolation functions
    kvs::Real32 m_dNdr[6]; ///< differencial interpolation functions
    kvs::Vec3 m_local; ///< local point (p, q, r)
    const kvs::UnstructuredVolumeObject* m_volume;

public:

    PrismCell( const kvs::UnstructuredVolumeObject* volume );

    kvs::Vec3 coord( const size_t index ) const { return m_coords[ index ]; }
    kvs::Vec3 value( const size_t index ) const { return m_values[ index ]; }
    kvs::Real32 N( const size_t index ) const { return m_N[ index ]; }
    kvs::Real32 dNdp( const size_t index ) const { return m_dNdp[ index ]; }
    kvs::Real32 dNdq( const size_t index ) const { return m_dNdq[ index ]; }
    kvs::Real32 dNdr( const size_t index ) const { return m_dNdr[ index ]; }

    void bind( const size_t index );
    void setLocalPoint( const kvs::Vec3 point );
    kvs::Vec3 localToGlobal( const kvs::Vec3 point );
    kvs::Vec3 globalToLocal( const kvs::Vec3 point );
    kvs::Mat3 gradient();
    kvs::Mat3 localGradient();
    kvs::Mat3 JacobiMatrix();
    void updateInterpolationFunctions( const kvs::Vec3 point );
};

} // end of namespace local
