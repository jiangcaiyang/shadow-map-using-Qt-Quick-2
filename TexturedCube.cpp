#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QQmlFile>
#include <QQuickWindow>
#include "View.h"
#include "TexturedCube.h"

#define CUBE_LENGTH         10.0
#define VERTEX_COUNT        36

///////////////////////////////////////////////////////////////////////////////
struct CommonVertex
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

class View;
class TexturedCubeRenderer: protected QOpenGLFunctions
{
public:
    TexturedCubeRenderer( TexturedCube* const cube ):
        m_cube( cube ),
        m_vertexBuffer( QOpenGLBuffer::VertexBuffer ),
        m_texture( QOpenGLTexture::Target2D )
    {
        initializeOpenGLFunctions( );
        // 创建着色器、顶点缓存以及纹理
        const char* vertexShaderSource =
                "attribute highp vec3 position;\
                attribute highp vec3 normal;\
        attribute highp vec2 texCoord;\
        uniform mat4 modelMatrix;\
        uniform mat4 viewMatrix;\
        uniform mat4 projectionMatrix;\
        varying highp vec2 v_texCoord;\
        varying highp vec3 v_normal;\
        void main( void )\
        {\
            gl_Position = projectionMatrix *\
                    viewMatrix *\
                    modelMatrix *\
                    vec4( position, 1.0 );\
            v_texCoord = texCoord;\
            v_normal = normal;\
        }";

        const char* fragmentShaderSource =
                "varying highp vec2 v_texCoord;\
                varying highp vec3 v_normal;\
        uniform highp sampler2D texture;\
        void main( void )\
        {\
            gl_FragColor = texture2D( texture, v_texCoord );\
        }";

        m_program.addShaderFromSourceCode( QOpenGLShader::Vertex,
                                           vertexShaderSource );
        m_program.addShaderFromSourceCode( QOpenGLShader::Fragment,
                                           fragmentShaderSource );
        m_program.link( );
        m_program.bind( );
        m_positionLoc = m_program.attributeLocation( "position" );
        m_normalLoc = m_program.attributeLocation( "normal" );
        m_texCoordLoc = m_program.attributeLocation( "texCoord" );
        m_modelMatrixLoc = m_program.uniformLocation( "modelMatrix" );
        m_viewMatrixLoc = m_program.uniformLocation( "viewMatrix" );
        m_projectionMatrixLoc = m_program.uniformLocation( "projectionMatrix" );
        m_textureLoc = m_program.uniformLocation( "texture" );
        m_program.setUniformValue( m_textureLoc, 0 );
        m_program.release( );

        // 设置顶点坐标
        qreal semi = m_cube->m_length / 2.0;
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

        m_vertices = new CommonVertex[VERTEX_COUNT];
        CommonVertex* v = m_vertices;

        // 前面
        v[0].set( basicVertices[6], normals[2], texCoords[0] );
        v[1].set( basicVertices[2], normals[2], texCoords[1] );
        v[2].set( basicVertices[5], normals[2], texCoords[2] );
        v[3].set( basicVertices[2], normals[2], texCoords[1] );
        v[4].set( basicVertices[1], normals[2], texCoords[3] );
        v[5].set( basicVertices[5], normals[2], texCoords[2] );

        // 后面
        v[6].set( basicVertices[4], normals[4], texCoords[0] );
        v[7].set( basicVertices[0], normals[4], texCoords[1] );
        v[8].set( basicVertices[7], normals[4], texCoords[2] );
        v[9].set( basicVertices[0], normals[4], texCoords[1] );
        v[10].set( basicVertices[3], normals[4], texCoords[3] );
        v[11].set( basicVertices[7], normals[4], texCoords[2] );

        // 上面
        v[12].set( basicVertices[2], normals[1], texCoords[0] );
        v[13].set( basicVertices[3], normals[1], texCoords[1] );
        v[14].set( basicVertices[1], normals[1], texCoords[2] );
        v[15].set( basicVertices[3], normals[1], texCoords[1] );
        v[16].set( basicVertices[0], normals[1], texCoords[3] );
        v[17].set( basicVertices[1], normals[1], texCoords[2] );

        // 下面
        v[18].set( basicVertices[7], normals[5], texCoords[0] );
        v[19].set( basicVertices[6], normals[5], texCoords[1] );
        v[20].set( basicVertices[4], normals[5], texCoords[2] );
        v[21].set( basicVertices[6], normals[5], texCoords[1] );
        v[22].set( basicVertices[5], normals[5], texCoords[3] );
        v[23].set( basicVertices[4], normals[5], texCoords[2] );

        // 左面
        v[24].set( basicVertices[7], normals[3], texCoords[0] );
        v[25].set( basicVertices[3], normals[3], texCoords[1] );
        v[26].set( basicVertices[6], normals[3], texCoords[2] );
        v[27].set( basicVertices[3], normals[3], texCoords[1] );
        v[28].set( basicVertices[2], normals[3], texCoords[3] );
        v[29].set( basicVertices[6], normals[3], texCoords[2] );

        // 右面
        v[30].set( basicVertices[5], normals[2], texCoords[0] );
        v[31].set( basicVertices[1], normals[2], texCoords[1] );
        v[32].set( basicVertices[4], normals[2], texCoords[2] );
        v[33].set( basicVertices[1], normals[2], texCoords[1] );
        v[34].set( basicVertices[0], normals[2], texCoords[3] );
        v[35].set( basicVertices[4], normals[2], texCoords[2] );

        m_vertexBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
        m_vertexBuffer.create( );
        m_vertexBuffer.bind( );
        m_vertexBuffer.allocate( v, sizeof( CommonVertex ) * VERTEX_COUNT );
    }
    ~TexturedCubeRenderer( void )
    {
        m_vertexBuffer.destroy( );
        m_texture.destroy( );
        delete []m_vertices;
    }
    void setVertexAttribute( int attributeLocation,
                             GLenum elementType,
                             quint32 elementSize,
                             quint32 offset )
    {
        m_program.enableAttributeArray( attributeLocation );
        m_program.setAttributeBuffer( attributeLocation,      // 位置
                                      elementType,            // 类型
                                      offset,                 // 偏移
                                      elementSize,            // 元大小
                                      sizeof( CommonVertex ) );// 迈
    }
    void loadTextureFromSource( const QUrl& source )
    {
        QString imagePath = QQmlFile::urlToLocalFileOrQrc( source );
        m_texture.setData( QImage( imagePath ).mirrored( ) );

        // 设置纹理滤波
        m_texture.setMinificationFilter( QOpenGLTexture::LinearMipMapLinear );
        m_texture.setMagnificationFilter( QOpenGLTexture::Linear );
    }
    void setLength( qreal length )
    {
        m_vertexBuffer.bind( );
        for ( int i = 0; i < VERTEX_COUNT; ++i )
        {
            m_vertices[i].position =
                    m_vertices[i].position.normalized( ) * length;
        }
        m_vertexBuffer.write( 0, m_vertices, sizeof( CommonVertex ) * VERTEX_COUNT );
        m_vertexBuffer.release( );
    }
    void render( void )
    {
        m_program.bind( );
        m_vertexBuffer.bind( );

        // 绘制box
        int offset = 0;
        setVertexAttribute( m_positionLoc, GL_FLOAT, 3, offset );
        offset += 3 * sizeof( GLfloat );
        setVertexAttribute( m_normalLoc, GL_FLOAT, 3, offset );
        offset += 3 * sizeof( GLfloat );
        setVertexAttribute( m_texCoordLoc, GL_FLOAT, 2, offset );

        QMatrix4x4 modelMatrix;
        modelMatrix.translate( m_cube->m_translate );
        m_program.setUniformValue( m_modelMatrixLoc, modelMatrix );
        m_program.setUniformValue( m_viewMatrixLoc, m_cube->m_view->viewMatrix( ) );
        m_program.setUniformValue( m_projectionMatrixLoc, m_cube->m_view->projectionMatrix( ) );

        m_texture.bind( );
        glDrawArrays( GL_TRIANGLES, 0, VERTEX_COUNT );
        m_texture.release( );
        m_vertexBuffer.release( );

        m_program.disableAttributeArray( m_positionLoc );
        m_program.disableAttributeArray( m_normalLoc );
        m_program.disableAttributeArray( m_texCoordLoc );

        m_program.release( );
    }
protected:
    // 同步的项目
    TexturedCube*           m_cube;

    QOpenGLShaderProgram    m_program;
    QOpenGLBuffer           m_vertexBuffer;
    QOpenGLTexture          m_texture;
    CommonVertex*           m_vertices;

    int m_positionLoc, m_normalLoc, m_texCoordLoc, m_modelMatrixLoc,
    m_viewMatrixLoc, m_projectionMatrixLoc, m_textureLoc;
};

///////////////////////////////////////////////////////////////////////////////
TexturedCube::TexturedCube( QObject* parent ): QObject( parent )
{
    m_length = CUBE_LENGTH;
    m_renderer = Q_NULLPTR;
    m_view = Q_NULLPTR;
}

void TexturedCube::initialize( void )
{
    m_renderer = new TexturedCubeRenderer( this );
}

void TexturedCube::render( void )
{
    m_renderer->render( );
}

void TexturedCube::sync( void )
{
    if ( m_sourceDirty )
    {
        m_renderer->loadTextureFromSource( m_source );
        m_sourceDirty = false;
    }

    if ( m_lengthDirty )
    {
        m_renderer->setLength( m_length );
        m_lengthDirty = false;
    }
}

void TexturedCube::release( void )
{
    delete m_renderer;
}

void TexturedCube::setLength( qreal length )
{
    if ( m_length == length ) return;
    m_length = length;
    emit lengthChanged( );
    m_lengthDirty = true;
    updateWindow( );
}

void TexturedCube::setSource( const QUrl& source )
{
    if ( m_source == source ) return;
    m_source = source;
    emit sourceChanged( );
    m_sourceDirty = true;
    updateWindow( );
}

void TexturedCube::setTranslate( const QVector3D& translate )
{
    if ( m_translate == translate ) return;
    m_translate = translate;
    emit translateChanged( );
    updateWindow( );
}

void TexturedCube::updateWindow( void )
{
    if ( m_view != Q_NULLPTR &&
         m_view->window( ) != Q_NULLPTR )
        m_view->window( )->update( );
}
