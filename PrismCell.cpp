/*****************************************************************************/
/**
 *  @file   PrismCell.cpp
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
#include "PrismCell.h"


namespace
{

kvs::Vec3 Mix( const kvs::Vec3 a, const kvs::Vec3 b, const float s )
{
    const float x = kvs::Math::Mix( a.x(), b.x(), s );
    const float y = kvs::Math::Mix( a.y(), b.y(), s );
    const float z = kvs::Math::Mix( a.z(), b.z(), s );
    return kvs::Vec3( x, y, z );
}

}


namespace local
{

PrismCell::PrismCell( const kvs::UnstructuredVolumeObject* volume ):
    m_volume( volume )
{
}

void PrismCell::bind( const size_t index )
{
    const kvs::UInt32* connections = m_volume->connections().data();
    const kvs::Real32* coords = m_volume->coords().data();
    const kvs::Real32* values = m_volume->values().asValueArray<kvs::Real32>().data();
    const size_t nnodes = 6;
    for ( size_t i = 0; i < nnodes; i++ )
    {
        const size_t id = connections[ nnodes * index + i ];
        m_coords[i] = kvs::Vec3( coords + 3 * id );
        m_values[i] = kvs::Vec3( values + 3 * id );
    }
}

void PrismCell::setLocalPoint( const kvs::Vec3 point )
{
    this->updateInterpolationFunctions( point );
}

kvs::Vec3 PrismCell::localToGlobal( const kvs::Vec3 point )
{
    kvs::Real32 x = 0;
    kvs::Real32 y = 0;
    kvs::Real32 z = 0;
    const size_t nnodes = 6;
    for ( size_t i = 0; i < nnodes; i++ )
    {
        x += m_N[i] * m_coords[i].x();
        y += m_N[i] * m_coords[i].y();
        z += m_N[i] * m_coords[i].z();
    }

    return kvs::Vec3( x, y, z );
}

kvs::Vec3 PrismCell::globalToLocal( const kvs::Vec3 point )
{
    const kvs::Vec3 X( point );

    const float TinyValue = static_cast<float>( 1.e-6 );
    const size_t MaxLoop = 100;
    kvs::Vec3 x0( 0.3f, 0.3f, 0.5f );
    for ( size_t i = 0; i < MaxLoop; i++ )
    {
        this->setLocalPoint( x0 );
        const kvs::Vec3 X0( this->localToGlobal( x0 ) );
        const kvs::Vec3 dX( X - X0 );

        const kvs::Mat3 J( this->JacobiMatrix() );
        const kvs::Vec3 dx = J.transposed().inverted() * dX;
        if ( dx.length() < TinyValue ) break; // Converged.

        x0 += dx;
    }

    return x0;
}

kvs::Mat3 PrismCell::gradient()
{
    const kvs::Mat3 t = this->localGradient();
    const kvs::Mat3 J = this->JacobiMatrix();

    kvs::Real32 det = 0.0f;
    const kvs::Mat3 T = 3.0f * J.inverted( &det ) * t;

    return kvs::Math::IsZero( det ) ? kvs::Mat3::Zero() : T;
}

kvs::Mat3 PrismCell::localGradient()
{
    const kvs::Real32 p = m_local.x();
    const kvs::Real32 q = m_local.y();
    const kvs::Real32 r = m_local.z();
    KVS_ASSERT( 0.0f <= p && p <= 1.0f );
    KVS_ASSERT( 0.0f <= q && q <= 1.0f );
    KVS_ASSERT( 0.0f <= r && r <= 1.0f );
    KVS_ASSERT( p + q <= 1.0f );

    const kvs::Vec3 v02 = ::Mix( this->value(0), this->value(2), q );
    const kvs::Vec3 v35 = ::Mix( this->value(3), this->value(5), q );
    const kvs::Vec3 v12 = ::Mix( this->value(1), this->value(2), q );
    const kvs::Vec3 v45 = ::Mix( this->value(4), this->value(5), q );
    const kvs::Vec3 x0 = ::Mix( v02, v35, r );
    const kvs::Vec3 x1 = ::Mix( v12, v45, r );

    const kvs::Vec3 v01 = ::Mix( this->value(0), this->value(1), p );
    const kvs::Vec3 v34 = ::Mix( this->value(3), this->value(4), p );
    const kvs::Vec3 v21 = ::Mix( this->value(2), this->value(1), p );
    const kvs::Vec3 v54 = ::Mix( this->value(5), this->value(4), p );
    const kvs::Vec3 y0 = ::Mix( v01, v34, r );
    const kvs::Vec3 y1 = ::Mix( v21, v54, r );

    const kvs::Real32 ratio = kvs::Math::IsZero( 1 - q ) ? 0.0f : p / ( 1 - q );
    const kvs::Vec3 z0 = ::Mix( v02, v12, ratio );
    const kvs::Vec3 z1 = ::Mix( v35, v45, ratio );

    const kvs::Real32 dx = 1 - q;
    const kvs::Real32 dy = 1 - p;
    const kvs::Real32 dz = 1;

    const kvs::Real32 dudx = ( x1.x() - x0.x() ) / dx;
    const kvs::Real32 dudy = ( y1.x() - y0.x() ) / dy;
    const kvs::Real32 dudz = ( z1.x() - z0.x() ) / dz;

    const kvs::Real32 dvdx = ( x1.y() - x0.y() ) / dx;
    const kvs::Real32 dvdy = ( y1.y() - y0.y() ) / dy;
    const kvs::Real32 dvdz = ( z1.y() - z0.y() ) / dz;

    const kvs::Real32 dwdx = ( x1.z() - x0.z() ) / dx;
    const kvs::Real32 dwdy = ( y1.z() - y0.z() ) / dy;
    const kvs::Real32 dwdz = ( z1.z() - z0.z() ) / dz;

    return kvs::Mat3(
        dudx, dvdx, dwdx,
        dudy, dvdy, dwdy,
        dudz, dvdz, dwdz );
}

kvs::Mat3 PrismCell::JacobiMatrix()
{
    kvs::Real32 dxdp = 0;
    kvs::Real32 dxdq = 0;
    kvs::Real32 dxdr = 0;
    kvs::Real32 dydp = 0;
    kvs::Real32 dydq = 0;
    kvs::Real32 dydr = 0;
    kvs::Real32 dzdp = 0;
    kvs::Real32 dzdq = 0;
    kvs::Real32 dzdr = 0;

    const size_t nnodes = 6;
    for ( size_t i = 0; i < nnodes; i++ )
    {
        dxdp += m_dNdp[i] * m_coords[i].x();
        dxdq += m_dNdq[i] * m_coords[i].x();
        dxdr += m_dNdr[i] * m_coords[i].x();

        dydp += m_dNdp[i] * m_coords[i].y();
        dydq += m_dNdq[i] * m_coords[i].y();
        dydr += m_dNdr[i] * m_coords[i].y();

        dzdp += m_dNdp[i] * m_coords[i].z();
        dzdq += m_dNdq[i] * m_coords[i].z();
        dzdr += m_dNdr[i] * m_coords[i].z();
    }

    return kvs::Mat3(
        dxdp, dydp, dzdp,
        dxdq, dydq, dzdq,
        dxdr, dydr, dzdr );
}

void PrismCell::updateInterpolationFunctions( const kvs::Vec3 point )
{
    // 0 <= p,q,r <= 1, p + q <= 1
    m_local = point;
    const float p = m_local.x();
    const float q = m_local.y();
    const float r = m_local.z();

    m_N[0] = ( 1 - p - q ) * r;
    m_N[1] = p * r;
    m_N[2] = q * r;
    m_N[3] = ( 1 - p - q ) * ( 1 - r );
    m_N[4] = p * ( 1 - r );
    m_N[5] = q * ( 1 - r );

    m_dNdp[0] = -r;
    m_dNdp[1] =  r;
    m_dNdp[2] =  0;
    m_dNdp[3] = -( 1 - r );
    m_dNdp[4] =  ( 1 - r );
    m_dNdp[5] =  0;

    m_dNdq[0] = -r;
    m_dNdq[1] =  0;
    m_dNdq[2] =  r;
    m_dNdq[3] = -( 1 - r );
    m_dNdq[4] =  0;
    m_dNdq[5] =  ( 1 - r );

    m_dNdr[0] =  ( 1 - p - q );
    m_dNdr[1] =  p;
    m_dNdr[2] =  q;
    m_dNdr[3] = -( 1 - p - q );
    m_dNdr[4] = -p;
    m_dNdr[5] = -q;
}

} // end of namespace local
