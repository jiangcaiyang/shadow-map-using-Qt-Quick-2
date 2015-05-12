#include <math.h>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QQmlFile>
#include "View.h"
#include "Plane.h"

#define VERTEX_COUNT    6
#define PLANE_LENGTH    25.0
#define TEXTURE_UNIT    GL_TEXTURE0
#define SHADOW_TEXTURE_UNIT GL_TEXTURE1

static void canonicalPosition( QVector3D& position )
{
    if ( !qFuzzyIsNull( position.x( ) ) )
        position.setX( position.x( ) / fabsf( position.x( ) ) );
    if ( !qFuzzyIsNull( position.y( ) ) )
        position.setY( position.y( ) / fabsf( position.y( ) ) );
    if ( !qFuzzyIsNull( position.z( ) ) )
        position.setZ( position.z( ) / fabsf( position.z( ) ) );
}

class PlaneRenderer: protected QOpenGLFunctions
{
    struct Vertex
    {
        void set( const QVector3D& _position, const QVector3D& _normal,
                  const QVector2D& _texCoord )
        {
            position = _position;
            normal = _normal;
            texCoord = _texCoord;
        }

        QVector3D               position;
        QVector3D               normal;
        QVector2D               texCoord;
    };
public:
    enum ShadowType
    {
        NoShadow = 0,// 以后依次递增
        SimpleShadow,
        PCFShadow
    };

    explicit PlaneRenderer( Plane* plane, ShadowType shadowType ):
        m_plane( plane ),
        m_shadowType( shadowType ),
        m_vertexBuffer( QOpenGLBuffer::VertexBuffer ),
        m_texture( QOpenGLTexture::Target2D )
    {
        initializeOpenGLFunctions( );

        // 根据创建的次数来创建着色器
        if ( s_count++ == 0 )
        {
            s_program = new QOpenGLShaderProgram;
            s_program->addShaderFromSourceFile( QOpenGLShader::Vertex,
                                                ":/Common.vert" );
            s_program->addShaderFromSourceFile( QOpenGLShader::Fragment,
                                                ":/Common.frag" );
            s_program->link( );
            s_program->bind( );
            s_positionLoc = s_program->attributeLocation( "position" );
            s_normalLoc = s_program->attributeLocation( "normal" );
            s_texCoordLoc = s_program->attributeLocation( "texCoord" );
            s_modelMatrixLoc = s_program->uniformLocation( "modelMatrix" );
            s_viewMatrixLoc = s_program->uniformLocation( "viewMatrix" );
            s_projectionMatrixLoc = s_program->uniformLocation( "projectionMatrix" );
            s_lightPositionLoc = s_program->uniformLocation( "lightPosition" );
            s_lightViewProjectionMatrixLoc =
                    s_program->uniformLocation( "lightViewProjectionMatrix" );
            s_modelViewNormalMatrixLoc =
                    s_program->uniformLocation( "modelViewNormalMatrix" );
            s_shadowTypeLoc = s_program->uniformLocation( "shadowType" );
            int textureLoc = s_program->uniformLocation( "texture" );
            int shadowLoc = s_program->uniformLocation( "shadowTexture" );
            s_program->setUniformValue( textureLoc,
                                        TEXTURE_UNIT - GL_TEXTURE0 );
            s_program->setUniformValue( shadowLoc,
                                        SHADOW_TEXTURE_UNIT - GL_TEXTURE0 );

            s_program->release( );
        }

        // 设置顶点坐标
        qreal semi = PLANE_LENGTH / 2.0;
        const QVector3D vertices[] =
        {
            QVector3D( semi, 0.0f, -semi ),
            QVector3D( semi, 0.0f, semi ),
            QVector3D( -semi, 0.0f, -semi ),
            QVector3D( -semi, 0.0f, semi )
        };

        const QVector2D texCoords[] =
        {
            QVector2D( 0.0, 0.0 ),
            QVector2D( 0.0, 1.0 ),
            QVector2D( 1.0, 0.0 ),
            QVector2D( 1.0, 1.0 )
        };

        m_vertices = new Vertex[VERTEX_COUNT];
        Vertex* v = m_vertices;

        v[0].set( vertices[2], QVector3D( 0.0f, 1.0f, 0.0f ), texCoords[1] );
        v[1].set( vertices[1], QVector3D( 0.0f, 1.0f, 0.0f ), texCoords[2] );
        v[2].set( vertices[0], QVector3D( 0.0f, 1.0f, 0.0f ), texCoords[0] );
        v[3].set( vertices[2], QVector3D( 0.0f, 1.0f, 0.0f ), texCoords[1] );
        v[4].set( vertices[3], QVector3D( 0.0f, 1.0f, 0.0f ), texCoords[3] );
        v[5].set( vertices[1], QVector3D( 0.0f, 1.0f, 0.0f ), texCoords[2] );

        m_vertexBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
        m_vertexBuffer.create( );
        m_vertexBuffer.bind( );
        m_vertexBuffer.allocate( v, VERTEX_COUNT * sizeof( Vertex ) );
        m_vertexBuffer.release( );

        // 设置纹理滤波
        m_texture.setMinificationFilter( QOpenGLTexture::LinearMipMapLinear );
        m_texture.setMagnificationFilter( QOpenGLTexture::Linear );
    }
    ~PlaneRenderer( void )
    {
        m_vertexBuffer.destroy( );
        m_texture.destroy( );
        delete []m_vertices;
        if ( --s_count == 0 )
        {
            delete s_program;
        }
    }
    void render( void )
    {
        s_program->bind( );
        m_vertexBuffer.bind( );

        // 绘制box
        int offset = 0;
        setVertexAttribute( s_positionLoc, GL_FLOAT, 3, offset );
        offset += 3 * sizeof( GLfloat );
        setVertexAttribute( s_normalLoc, GL_FLOAT, 3, offset );
        offset += 3 * sizeof( GLfloat );
        setVertexAttribute( s_texCoordLoc, GL_FLOAT, 2, offset );

        // 摄像机的MVP矩阵
        QMatrix4x4& viewMatrix = m_plane->m_view->viewMatrix( );
        s_program->setUniformValue( s_modelMatrixLoc, m_modelMatrix );
        s_program->setUniformValue( s_viewMatrixLoc, viewMatrix );
        s_program->setUniformValue( s_projectionMatrixLoc, m_plane->m_view->projectionMatrix( ) );
        s_program->setUniformValue( s_modelViewNormalMatrixLoc,
                                    ( viewMatrix * m_modelMatrix ).normalMatrix( ) );
        // 是否启用实时阴影
        //s_program->setUniformValue( s_shadowTypeLoc, m_shadowType );

        m_texture.bind( );
        if ( m_shadowType != NoShadow )
        {
            s_program->setUniformValue( s_lightPositionLoc, m_plane->m_view->lightPosition( ) );
            s_program->setUniformValue( s_lightViewProjectionMatrixLoc,
                                        m_plane->m_view->lightViewProjectionMatrix( ) );

            glActiveTexture( SHADOW_TEXTURE_UNIT );
            glBindTexture( GL_TEXTURE_2D, m_plane->m_view->shadowTexture( ) );
            glDrawArrays( GL_TRIANGLES, 0, VERTEX_COUNT );
            glActiveTexture( TEXTURE_UNIT );
        }
        else
        {
            glDrawArrays( GL_TRIANGLES, 0, VERTEX_COUNT );
        }
        m_texture.release( );
        m_vertexBuffer.release( );

        s_program->release( );
    }
    void renderShadow( void )
    {
        m_vertexBuffer.bind( );
        QOpenGLShaderProgram* depthProgram = m_plane->m_view->depthProgram( );
        depthProgram->enableAttributeArray( "position" );
        depthProgram->setAttributeBuffer(
                    "position",             // 位置
                    GL_FLOAT,               // 类型
                    0,                      // 偏移
                    3,                      // 元大小
                    sizeof( Vertex ) );     // 迈

        depthProgram->setUniformValue( "modelMatrix", m_modelMatrix );
        glDrawArrays( GL_TRIANGLES, 0, VERTEX_COUNT );
        m_vertexBuffer.release( );
    }
    void resize( qreal length )
    {
        qreal semi = length / 2.0;
        m_vertexBuffer.bind( );
        Vertex* v = (Vertex*)m_vertexBuffer.map( QOpenGLBuffer::WriteOnly );
        for ( int i = 0; i < VERTEX_COUNT; ++i )
        {
            canonicalPosition( v[i].position );
            v[i].position *= semi;
        }
        m_vertexBuffer.unmap( );
        m_vertexBuffer.release( );
    }
    void setVertexAttribute( int attributeLocation,
                             GLenum elementType,
                             quint32 elementSize,
                             quint32 offset )
    {
        s_program->enableAttributeArray( attributeLocation );
        s_program->setAttributeBuffer( attributeLocation,      // 位置
                                       elementType,            // 类型
                                       offset,                 // 偏移
                                       elementSize,            // 元大小
                                       sizeof( Vertex ) );     // 迈
    }
    void loadTextureFromSource( const QUrl& source )
    {
        QString imagePath = QQmlFile::urlToLocalFileOrQrc( source );
        m_texture.setData( QImage( imagePath ).mirrored( ) );

        // 设置纹理滤波
        m_texture.setMinificationFilter( QOpenGLTexture::LinearMipMapLinear );
        m_texture.setMagnificationFilter( QOpenGLTexture::Linear );
    }
    void translate( const QVector3D& translate )
    {
        m_modelMatrix.setToIdentity( );
        m_modelMatrix.translate( translate );
    }
protected:
    Plane*                  m_plane;

    QMatrix4x4              m_modelMatrix;
    ShadowType              m_shadowType;
    QOpenGLBuffer           m_vertexBuffer;
    QOpenGLTexture          m_texture;
    Vertex*                 m_vertices;

    static QOpenGLShaderProgram* s_program;
    static int s_positionLoc, s_normalLoc,
    s_texCoordLoc, s_modelMatrixLoc,
    s_viewMatrixLoc, s_projectionMatrixLoc,
    s_lightViewProjectionMatrixLoc,
    s_lightPositionLoc, s_modelViewNormalMatrixLoc, s_shadowTypeLoc;
    static int              s_count;        // 计数
};

QOpenGLShaderProgram* PlaneRenderer::s_program = Q_NULLPTR;
int PlaneRenderer::s_positionLoc,
PlaneRenderer::s_normalLoc,
PlaneRenderer::s_texCoordLoc,
PlaneRenderer::s_modelMatrixLoc,
PlaneRenderer::s_viewMatrixLoc,
PlaneRenderer::s_projectionMatrixLoc,
PlaneRenderer::s_lightViewProjectionMatrixLoc,
PlaneRenderer::s_lightPositionLoc,
PlaneRenderer::s_modelViewNormalMatrixLoc,
PlaneRenderer::s_shadowTypeLoc,
PlaneRenderer::s_count = 0;

Plane::Plane( QObject* parent ): QObject( parent )
{
    m_length = PLANE_LENGTH;
    m_lengthIsDirty = false;
    m_sourceIsDirty = false;
}

void Plane::initialize( void )
{
    m_renderer = new PlaneRenderer( this, PlaneRenderer::SimpleShadow );
}

void Plane::render( void )
{
    m_renderer->render( );
}

void Plane::renderShadow( void )
{
    m_renderer->renderShadow( );
}

void Plane::sync( void )
{
    if ( m_lengthIsDirty )
    {
        m_renderer->resize( m_length );
        m_lengthIsDirty = false;
    }
    if ( m_sourceIsDirty )
    {
        m_renderer->loadTextureFromSource( m_source );
        m_sourceIsDirty = false;
    }
    if ( m_translateIsDirty )
    {
        m_renderer->translate( m_translate );
        m_translateIsDirty = false;
    }
}

void Plane::release( void )
{
    delete m_renderer;
}

void Plane::setLength( qreal length )
{
    if ( m_length == length ) return;
    m_length = length;
    emit lengthChanged( );
    m_lengthIsDirty = true;
}

void Plane::setSource( const QUrl& source )
{
    if ( m_source == source ) return;
    m_source = source;
    emit sourceChanged( );
    m_sourceIsDirty = true;
}

void Plane::setTranslate( const QVector3D& translate )
{
    if ( m_translate == translate ) return;
    m_translate = translate;
    emit translateChanged( );
    m_translateIsDirty = true;
}

