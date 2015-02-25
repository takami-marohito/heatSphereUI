#ifndef LOAD_UCDFILE_H_INCLUDE
#define LOAD_UCDFILE_H_INCLUDE


#include <kvs/FileFormatBase>

#include <kvs/AnyValueArray>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <string>

namespace takami
{

class LoadUcdFile : public kvs::FileFormatBase
{
public:

    enum WritingDataType
    {
        Ascii = 0,
        ExternalAscii,
        ExternalBinary
    };
protected:

    WritingDataType              m_writing_type;  ///< writing data type
    static std::string                 m_cell_type;     ///< cell type
    bool                         m_has_label;     ///< data label is specified or not
    std::string                  m_label;         ///< data label
    static size_t                       m_veclen;        ///< vector length
    static size_t                       m_nnodes;        ///< number of nodes
    static size_t                       m_ncells;        ///< number of cells
    bool                         m_has_min_value; ///< min. value is specified or not
    bool                         m_has_max_value; ///< max. value is specified or not
    double                       m_min_value;     ///< min. value
    double                       m_max_value;     ///< max. value
    static kvs::AnyValueArray           m_values;        ///< field value array
    static kvs::ValueArray<kvs::Real32> m_coords;        ///< coordinate value array
    static kvs::ValueArray<kvs::UInt32> m_connections;   ///< connection id array
 public:
    LoadUcdFile();
    LoadUcdFile( const std::string& filename );
    LoadUcdFile( const std::string& filename , char cell_type);
    virtual ~LoadUcdFile();

public:

    const std::string& cellType( void ) const;

    const bool hasLabel( void ) const;

    const std::string& label( void ) const;

    const size_t veclen( void ) const;

    const size_t nnodes( void ) const;

    const size_t ncells( void ) const;

    const bool hasMinValue( void ) const;

    const bool hasMaxValue( void ) const;

    const double minValue( void ) const;

    const double maxValue( void ) const;

    const kvs::AnyValueArray& values( void ) const;

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::UInt32>& connections( void ) const;

public:

    void setWritingDataType( const WritingDataType writing_type );

    void setCellType( const std::string& cell_type );

    void setLabel( const std::string& label );

    void setVeclen( const size_t veclen );

    void setNNodes( const size_t nnodes );

    void setNCells( const size_t ncells );

    void setMinValue( const double min_value );

    void setMaxValue( const double max_value );

    void setValues( const kvs::AnyValueArray& values );

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setConnections( const kvs::ValueArray<kvs::UInt32>& connections );

public:

    bool read( const std::string& filename );

    bool write( const std::string& filename );

    void read_tetra_file( const std::string& filename );

    void read_prism_file( const std::string& filename );

 private:
    char takami_cell_type;
};



} // end of namespace takami



#endif
