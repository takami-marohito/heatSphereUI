#include "Qedge.h"
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

class EdgeMap
{
public:

    typedef kvs::UInt32 Key;
    typedef std::pair<kvs::UInt32,kvs::UInt32> Value; // edge = (id0,id1)
    typedef std::multimap<Key,Value> Bucket;

private:

    size_t m_nnodes;
    Bucket m_bucket;

public:

    EdgeMap( const size_t nnodes ): m_nnodes( nnodes ) {}

    const Bucket& bucket() const { return m_bucket; }

    void insert( const kvs::UInt32 v0, const kvs::UInt32 v1 )
    {
        const Key key = ( v0 + v1 ) % kvs::UInt32( m_nnodes );

        Bucket::iterator e = m_bucket.find( key );
        Bucket::const_iterator last = m_bucket.end();
        if ( e != last )
        {
            Bucket::const_iterator upper = m_bucket.upper_bound( key );
            while ( e != upper )
            {
                if ( ( e->second.first == v0 && e->second.second == v1 ) ||
                     ( e->second.first == v1 && e->second.second == v0 ) )
                {
                    // The edge has been already inserted in the bucket.
                    return;
                }
                e++;
            }
        }

        m_bucket.insert( std::make_pair( key, std::make_pair( v0, v1 ) ) );
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

kvs::Vec3 GetValue( const kvs::UnstructuredVolumeObject* volume, const size_t index )
{
    const kvs::Real32* values = volume->values().asValueArray<kvs::Real32>().data();
    return kvs::Vec3( values + index * 3 );
}

kvs::Mat3 VelocityGradientTensor( const kvs::Vec3 x0, const kvs::Vec3 x1, const kvs::Vec3 v0, const kvs::Vec3 v1 )
{
   const kvs::Vec3 dx = x1 - x0;
   const kvs::Vec3 dv = v1 - v0;
   const kvs::Mat3 T(
       dv.x() / dx.x(), dv.x() / dx.y(), dv.x() / dx.z(),
       dv.y() / dx.x(), dv.y() / dx.y(), dv.y() / dx.z(),
       dv.z() / dx.x(), dv.z() / dx.y(), dv.z() / dx.z()
   );

   return T;
}

kvs::Real32 SecondInvariant( const kvs::Mat3 T )
{
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

Qedge::Qedge( const kvs::UnstructuredVolumeObject* volume ):
    kvs::FilterBase(),
    kvs::UnstructuredVolumeObject()
{
    this->exec( volume );
}

Qedge::SuperClass* Qedge::exec( const kvs::ObjectBase* object )
{
    const kvs::UnstructuredVolumeObject* volume = ::Cast( object );
    if ( !volume ) { return NULL; }

    const kvs::UInt32* connections = volume->connections().data();
    const size_t ncells = volume->numberOfCells();
    const size_t nnodes = volume->numberOfNodes();

    ::EdgeMap edge_map( nnodes );
    for ( size_t i = 0; i < ncells; i++ )
    {
        const kvs::UInt32 id0 = *( connections++ );
        const kvs::UInt32 id1 = *( connections++ );
        const kvs::UInt32 id2 = *( connections++ );
        const kvs::UInt32 id3 = *( connections++ );
        const kvs::UInt32 id4 = *( connections++ );
        const kvs::UInt32 id5 = *( connections++ );

        edge_map.insert( id0, id1 );
        edge_map.insert( id1, id2 );
        edge_map.insert( id2, id0 );
        edge_map.insert( id3, id4 );
        edge_map.insert( id4, id5 );
        edge_map.insert( id5, id3 );
        edge_map.insert( id0, id3 );
        edge_map.insert( id1, id4 );
        edge_map.insert( id2, id5 );
    }

    ::NodeMap node_map( nnodes );
    ::EdgeMap::Bucket::const_iterator e = edge_map.bucket().begin();
    ::EdgeMap::Bucket::const_iterator last = edge_map.bucket().end();
    while ( e != last )
    {
        const kvs::UInt32 id0 = e->second.first;
        const kvs::UInt32 id1 = e->second.second;

        const kvs::Vec3 x0 = ::GetCoord( volume, id0 );
        const kvs::Vec3 x1 = ::GetCoord( volume, id1 );

        if ( !kvs::Math::IsZero( ( x1.x() - x0.x() ) ) &&
             !kvs::Math::IsZero( ( x1.y() - x0.y() ) ) &&
             !kvs::Math::IsZero( ( x1.z() - x0.z() ) ) )
        {
            const kvs::Vec3 v0 = ::GetValue( volume, id0 );
            const kvs::Vec3 v1 = ::GetValue( volume, id1 );

            const kvs::Mat3 t = ::VelocityGradientTensor( x0, x1, v0, v1 );
            const kvs::Real32 qvalue = ::SecondInvariant( t );
            const kvs::Real32 distance = ( x1 - x0 ).length() * 0.5f;

            node_map.insert( id0, qvalue, distance );
            node_map.insert( id1, qvalue, distance );
        }

        e++;
    }

    SuperClass::shallowCopy( *volume );
    SuperClass::setVeclen( 1 );
    SuperClass::setValues( kvs::AnyValueArray( ::InverseDistanceWeighting( node_map ) ) );
    SuperClass::updateMinMaxValues();

    return this;
}

} // end of namespace local
