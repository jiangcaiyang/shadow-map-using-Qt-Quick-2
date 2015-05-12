#include <math.h>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QQmlFile>
#include "View.h"
#include "Cube.h"

#define VERTEX_COUNT   36
#define CUBE_LENGTH    25.0
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

class CubeRenderer: protected QOpenGLFunctions
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

    explicit CubeRenderer( Cube* plane, ShadowType shadowType ):
        m_cube( plane ),
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
            s_lightViewProjectionMatrixLoc = s_program->uniformLocation( "lightViewProjectionMatrix" );
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
        qreal semi = CUBE_LENGTH / 2.0;
        const QVector3D basicVertices[] =
        {
            QVector3D( semi, -semi, semi ),
            QVector3D( semi, -semi, -semi ),
            QVector3D( -semi, -semi, -semi ),
            QVector3D( -semi, -semi, semi ),
            QVector3D( semi, semi, semi ),
            QVector3D( semi, semi, -semi ),
            QVector3D( -semi, semi, -semi ),
            QVector3D( -semi, semi, semi )
        };

        const QVector3D normals[] =
        {
            QVector3D( 1.0, 0.0, 0.0 ),
            QVector3D( 0.0, 1.0, 0.0 ),
            QVector3D( 0.0, 0.0, 1.0 ),
            QVector3D( -1.0, 0.0, 0.0 ),
            QVector3D( 0.0, -1.0, 0.0 ),
            QVector3D( 0.0, 0.0, -1.0 )
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

        // 前面
        v[0].set( basicVertices[7], normals[2], texCoords[2] );
        v[1].set( basicVertices[3], normals[2], texCoords[0] );
        v[2].set( basicVertices[0], normals[2], texCoords[1] );
        v[3].set( basicVertices[4], normals[2], texCoords[3] );
        v[4].set( basicVertices[7], normals[2], texCoords[2] );
        v[5].set( basicVertices[0], normals[2], texCoords[1] );

        // 后面
        v[6].set( basicVertices[5], normals[5], texCoords[2] );
        v[7].set( basicVertices[2], normals[5], texCoords[1] );
        v[8].set( basicVertices[6], normals[5], texCoords[3] );
        v[9].set( basicVertices[5], normals[5], texCoords[2] );
        v[10].set( basicVertices[1], normals[5], texCoords[0] );
        v[11].set( basicVertices[2], normals[5], texCoords[1] );

        // 上面
        v[12].set( basicVertices[4], normals[1], texCoords[2] );
        v[13].set( basicVertices[5], normals[1], texCoords[3] );
        v[14].set( basicVertices[6], normals[1], texCoords[1] );
        v[15].set( basicVertices[4], normals[1], texCoords[2] );
        v[16].set( basicVertices[6], normals[1], texCoords[1] );
        v[17].set( basicVertices[7], normals[1], texCoords[0] );

        // 下面
        v[18].set( basicVertices[0], normals[4], texCoords[3] );
        v[19].set( basicVertices[2], normals[4], texCoords[0] );
        v[20].set( basicVertices[1], normals[4], texCoords[1] );
        v[21].set( basicVertices[0], normals[4], texCoords[3] );
        v[22].set( basicVertices[3], normals[4], texCoords[2] );
        v[23].set( basicVertices[2], normals[4], texCoords[0] );

        // 左面
        v[24].set( basicVertices[2], normals[3], texCoords[0] );
        v[25].set( basicVertices[3], normals[3], texCoords[1] );
        v[26].set( basicVertices[7], normals[3], texCoords[3] );
        v[27].set( basicVertices[2], normals[3], texCoords[0] );
        v[28].set( basicVertices[7], normals[3], texCoords[3] );
        v[29].set( basicVertices[6], normals[3], texCoords[2] );

        // 右面
        v[30].set( basicVertices[4], normals[0], texCoords[2] );
        v[31].set( basicVertices[1], normals[0], texCoords[1] );
        v[32].set( basicVertices[5], normals[0], texCoords[3] );
        v[33].set( basicVertices[1], normals[0], texCoords[1] );
        v[34].set( basicVertices[4], normals[0], texCoords[2] );
        v[35].set( basicVertices[0], normals[0], texCoords[0] );

        m_vertexBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
        m_vertexBuffer.create( );
        m_vertexBuffer.bind( );
        m_vertexBuffer.allocate( v, VERTEX_COUNT * sizeof( Vertex ) );
        m_vertexBuffer.release( );

        // 设置纹理滤波
        m_texture.setMinificationFilter( QOpenGLTexture::LinearMipMapLinear );
        m_texture.setMagnificationFilter( QOpenGLTexture::Linear );
    }
    ~CubeRenderer( void )
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
        QMatrix4x4& viewMatrix = m_cube->m_view->viewMatrix( );
        s_program->setUniformValue( s_modelMatrixLoc, m_modelMatrix );
        s_program->setUniformValue( s_viewMatrixLoc, viewMatrix );
        s_program->setUniformValue( s_projectionMatrixLoc, m_cube->m_view->projectionMatrix( ) );
        s_program->setUniformValue( s_modelViewNormalMatrixLoc,
                                    ( viewMatrix * m_modelMatrix ).normalMatrix( ) );

        // 是否启用实时阴影
        //s_program->setUniformValue( s_shadowTypeLoc, m_shadowType );

        m_texture.bind( );
        if ( m_shadowType != NoShadow )
        {
            s_program->setUniformValue( s_lightPositionLoc, m_cube->m_view->lightPosition( ) );
            s_program->setUniformValue( s_lightViewProjectionMatrixLoc,
                                        m_cube->m_view->lightViewProjectionMatrix( ) );

            glActiveTexture( SHADOW_TEXTURE_UNIT );
            glBindTexture( GL_TEXTURE_2D, m_cube->m_view->shadowTexture( ) );
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
        QOpenGLShaderProgram* depthProgram = m_cube->m_view->depthProgram( );
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
    Cube*                   m_cube;

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

QOpenGLShaderProgram* CubeRenderer::s_program = Q_NULLPTR;
int CubeRenderer::s_positionLoc,
CubeRenderer::s_normalLoc,
CubeRenderer::s_texCoordLoc,
CubeRenderer::s_modelMatrixLoc,
CubeRenderer::s_viewMatrixLoc,
CubeRenderer::s_projectionMatrixLoc,
CubeRenderer::s_lightViewProjectionMatrixLoc,
CubeRenderer::s_lightPositionLoc,
CubeRenderer::s_modelViewNormalMatrixLoc,
CubeRenderer::s_shadowTypeLoc,
CubeRenderer::s_count = 0;

Cube::Cube( QObject* parent ): QObject( parent )
{
    m_length = CUBE_LENGTH;
    m_lengthIsDirty = false;
    m_sourceIsDirty = false;
    m_translateIsDirty = false;
}

void Cube::initialize( void )
{
    m_renderer = new CubeRenderer( this, CubeRenderer::SimpleShadow );
}

void Cube::render( void )
{
    m_renderer->render( );
}

void Cube::renderShadow( void )
{
    m_renderer->renderShadow( );
}

void Cube::sync( void )
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

void Cube::release( void )
{
    delete m_renderer;
}

void Cube::setLength( qreal length )
{
    if ( m_length == length ) return;
    m_length = length;
    emit lengthChanged( );
    m_lengthIsDirty = true;
}

void Cube::setSource( const QUrl& source )
{
    if ( m_source == source ) return;
    m_source = source;
    emit sourceChanged( );
    m_sourceIsDirty = true;
}

void Cube::setTranslate( const QVector3D& translate )
{
    if ( m_translate == translate ) return;
    m_translate = translate;
    emit translateChanged( );
    m_translateIsDirty = true;
}
