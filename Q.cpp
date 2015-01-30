#include "Q.h"
#include <map>
#include <kvs/Type>
#include <kvs/UnstructuredVolumeObject>


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

    if ( uvolume->veclen() != 3 )
    {
        kvsMessageError("Input object is not a vector volume.");
        return NULL;
    }

    return uvolume;
}

class Tensor
{
    const kvs::UnstructuredVolumeObject* m_object;
    kvs::Vec3 m_v[6];

public:

    Tensor( const kvs::UnstructuredVolumeObject* object ):
        m_object( object ) {}

    void bind( const size_t index )
    {
        const kvs::UInt32* connections = m_object->connections().data();
        const size_t id0 = connections[ 6 * index + 0 ];
        const size_t id1 = connections[ 6 * index + 1 ];
        const size_t id2 = connections[ 6 * index + 2 ];
        const size_t id3 = connections[ 6 * index + 3 ];
        const size_t id4 = connections[ 6 * index + 4 ];
        const size_t id5 = connections[ 6 * index + 5 ];

        const kvs::Real32* values = m_object->values().asValueArray<kvs::Real32>().data();
        m_v[0] = kvs::Vec3( values + id0 * 3 );
        m_v[1] = kvs::Vec3( values + id1 * 3 );
        m_v[2] = kvs::Vec3( values + id2 * 3 );
        m_v[3] = kvs::Vec3( values + id3 * 3 );
        m_v[4] = kvs::Vec3( values + id4 * 3 );
        m_v[5] = kvs::Vec3( values + id5 * 3 );
    }

    kvs::Mat3 get() const
    {
        const kvs::Real32 ratio = 1.0f / 3.0f;

        const kvs::Vec3 v02 = this->mix( m_v[0], m_v[2], ratio );
        const kvs::Vec3 v35 = this->mix( m_v[3], m_v[5], ratio );
        const kvs::Vec3 v12 = this->mix( m_v[1], m_v[2], ratio );
        const kvs::Vec3 v45 = this->mix( m_v[4], m_v[5], ratio );
        const kvs::Vec3 x0 = this->mix( v02, v35, 0.5f );
        const kvs::Vec3 x1 = this->mix( v12, v45, 0.5f );

        const kvs::Vec3 v01 = this->mix( m_v[0], m_v[1], ratio );
        const kvs::Vec3 v34 = this->mix( m_v[3], m_v[4], ratio );
        const kvs::Vec3 v21 = this->mix( m_v[2], m_v[1], ratio );
        const kvs::Vec3 v54 = this->mix( m_v[5], m_v[4], ratio );
        const kvs::Vec3 y0 = this->mix( v01, v34, 0.5f );
        const kvs::Vec3 y1 = this->mix( v21, v54, 0.5f );

        const kvs::Vec3 z0 = this->mix( v01, v12, 0.5f );
        const kvs::Vec3 z1 = this->mix( v35, v45, 0.5f );

        const kvs::Real32 dx = 2.0f / 3.0f;
        const kvs::Real32 dy = 2.0f / 3.0f;
        const kvs::Real32 dz = 1.0f;

        const kvs::Real32 dudx = ( x1.x() - x0.x() ) / dx;
        const kvs::Real32 dudy = ( y1.x() - y0.x() ) / dy;
        const kvs::Real32 dudz = ( z1.x() - z0.x() ) / dz;

        const kvs::Real32 dvdx = ( x1.y() - x0.y() ) / dx;
        const kvs::Real32 dvdy = ( y1.y() - y0.y() ) / dy;
        const kvs::Real32 dvdz = ( z1.y() - z0.y() ) / dz;

        const kvs::Real32 dwdx = ( x1.z() - x0.z() ) / dx;
        const kvs::Real32 dwdy = ( y1.z() - y0.z() ) / dy;
        const kvs::Real32 dwdz = ( z1.z() - z0.z() ) / dz;

        const kvs::Mat3 T(
            dudx, dudy, dudz,
            dvdx, dvdy, dvdz,
            dwdx, dwdy, dwdz );

        return T;
    }

    kvs::Real32 q() const
    {
        const kvs::Mat3 T = this->get();
        const kvs::Real32 t00 = T[0][0];
        const kvs::Real32 t11 = T[1][1];
        const kvs::Real32 t22 = T[2][2];
        const kvs::Real32 t01 = T[0][1];
        const kvs::Real32 t10 = T[1][0];
        const kvs::Real32 t12 = T[1][2];
        const kvs::Real32 t21 = T[2][1];
        const kvs::Real32 t02 = T[0][2];
        const kvs::Real32 t20 = T[2][0];
        return t00 * t11 + t11 * t22 + t22 * t00 - t01 * t10 - t12 * t21 - t02 * t20;
    }

private:

    kvs::Vec3 mix( const kvs::Vec3 a, const kvs::Vec3 b, const float s ) const
    {
        const float x = kvs::Math::Mix( a.x(), b.x(), s );
        const float y = kvs::Math::Mix( a.y(), b.y(), s );
        const float z = kvs::Math::Mix( a.z(), b.z(), s );
        return kvs::Vec3( x, y, z );
    }
};

class NodeMap
{
public:

    typedef std::pair<kvs::Real32,kvs::Real32> Value; // (Q, distance)
    typedef std::vector<std::vector<Value> > Bucket;

private:

    Bucket m_bucket;

public:

    NodeMap( const size_t nnodes ) { m_bucket.resize( nnodes ); }

    const Bucket& bucket() const { return m_bucket; }

    void insert( const kvs::UInt32 index, const kvs::Real32 qvalue, const kvs::Real32 distance )
    {
        m_bucket[ index ].push_back( std::make_pair( qvalue, distance ) );
    }
};

kvs::Vec3 GetCoord( const kvs::UnstructuredVolumeObject* volume, const size_t index )
{
    const kvs::Real32* coords = volume->coords().data();
    return kvs::Vec3( coords + index * 3 );
}

kvs::ValueArray<kvs::Real32> InverseDistanceWeighting( const NodeMap& node_map )
{
    const size_t nnodes = node_map.bucket().size();
    kvs::ValueArray<kvs::Real32> values( nnodes );
    for ( size_t i = 0; i < nnodes; i++ )
    {
        const size_t n = node_map.bucket().at(i).size();

        float w = 0.0f;
        for ( size_t j = 0; j < n; j++ )
        {
            const float d = node_map.bucket().at(i).at(j).second;
            w += 1.0f / d;
        }

        kvs::Real32 value = 0.0f;
        for ( size_t j = 0; j < n; j++ )
        {
            const kvs::Real32 q = node_map.bucket().at(i).at(j).first;
            const kvs::Real32 d = node_map.bucket().at(i).at(j).second;
            value += ( ( 1.0f / d ) / w ) * q;
        }
        value /= n;
        values[i] = value;
    }

    return values;
}

}

namespace local
{

Q::Q( const kvs::UnstructuredVolumeObject* volume ):
    kvs::FilterBase(),
    kvs::UnstructuredVolumeObject()
{
    this->exec( volume );
}

Q::SuperClass* Q::exec( const kvs::ObjectBase* object )
{
    const kvs::UnstructuredVolumeObject* volume = ::Cast( object );
    if ( !volume ) { return NULL; }

    const kvs::UInt32* connections = volume->connections().data();
    const size_t ncells = volume->numberOfCells();
    const size_t nnodes = volume->numberOfNodes();

    ::Tensor tensor( volume );
    ::NodeMap node_map( nnodes );
    for ( size_t i = 0; i < ncells; i++ )
    {
        tensor.bind( i );
        const kvs::Real32 Q = tensor.q();

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

        node_map.insert( id0, Q, d0 );
        node_map.insert( id1, Q, d1 );
        node_map.insert( id2, Q, d2 );
        node_map.insert( id3, Q, d3 );
        node_map.insert( id4, Q, d4 );
        node_map.insert( id5, Q, d5 );
    }

    SuperClass::shallowCopy( *volume );
    SuperClass::setVeclen( 1 );
    SuperClass::setValues( kvs::AnyValueArray( ::InverseDistanceWeighting( node_map ) ) );
    SuperClass::updateMinMaxValues();

    return this;
}

} // end of namespace local
