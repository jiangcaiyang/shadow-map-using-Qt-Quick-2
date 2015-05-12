#include <QOpenGLFunctions>
#include <QQmlFile>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include "Cube.h"
#include "Plane.h"
#include "TexturedCube.h"
#include "View.h"

#define FBO_WIDTH       1024
#define FBO_HEIGHT      1024

///////////////////////////////////////////////////////////////////////////////
View::View( QQuickItem* parent ): QQuickItem( parent )
{
    m_position = QVector3D( 0.0f, 0.0f, 50.0f );
    m_lookAt = QVector3D( 0.0f, 0.0f, 0.0f );
    m_up = QVector3D( 0.0f, 1.0f, 0.0f );

    m_fieldOfView = 45.0;
    m_aspectRatio = 16.0 / 9.0;
    m_nearPlane = 0.5;
    m_farPlane = 500.0;

    m_initialized = false;
    m_viewMatrixDirty = false;
    m_projectionMatrixDirty = false;
    m_projectionMatrixDirty = false;

    m_FBO = Q_NULLPTR;
    m_depthProgram = Q_NULLPTR;

    connect( this, SIGNAL( windowChanged( QQuickWindow* ) ),
             this, SLOT( onWindowChanged( QQuickWindow* ) ) );
}

View::~View( void )
{

}


void View::onWindowChanged( QQuickWindow* win )
{
    if ( win != Q_NULLPTR )
    {
        connect( win, SIGNAL( beforeSynchronizing( ) ),
                 this, SLOT( sync( ) ), Qt::DirectConnection );
        connect( win, SIGNAL( sceneGraphInvalidated( ) ),
                 this, SLOT( cleanup( ) ), Qt::DirectConnection );
        win->setClearBeforeRendering( false );
    }
}

void View::render( void )
{
    renderShadow( );

    QRectF sceneRect = boundingRect( );
    QOpenGLFunctions* f = window( )->openglContext( )->functions( );
    f->glViewport( sceneRect.x( ),
                   sceneRect.y( ),
                   sceneRect.width( ),
                   sceneRect.height( ) );
    f->glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
    f->glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    foreach ( QObject* object, m_data )
    {
        Cube* cube = qobject_cast<Cube*>( object );
        Plane* plane = qobject_cast<Plane*>( object );
        TexturedCube* texturedCube = qobject_cast<TexturedCube*>( object );

        if ( cube != Q_NULLPTR ) cube->render( );
        else if ( plane != Q_NULLPTR ) plane->render( );
        else if ( texturedCube != Q_NULLPTR ) texturedCube->render( );
    }

    window( )->resetOpenGLState( );
}

void View::sync( void )
{
    if ( !m_initialized ) initialize( );

    if ( m_viewMatrixDirty )
    {
        m_viewMatrix = m_pendingViewMatrix;
        m_viewMatrixDirty = false;
    }

    if ( m_projectionMatrixDirty )
    {
        m_projectionMatrix = m_pendingProjectionMatrix;
        QMatrix4x4 lightViewMatrix;
        lightViewMatrix.lookAt( m_lightPosition,
                                QVector3D( 0, 0, 0 ),
                                QVector3D( 0, 1, 0 ) );
        m_lightViewProjectionMatrix = m_projectionMatrix * lightViewMatrix;
        m_projectionMatrixDirty = false;
    }

    if ( m_lightPositionDirty )
    {
        QMatrix4x4 lightViewMatrix;
        lightViewMatrix.lookAt( m_lightPosition,
                                QVector3D( 0, 0, 0 ),
                                QVector3D( 0, 1, 0 ) );
        m_lightViewProjectionMatrix = m_projectionMatrix * lightViewMatrix;
        m_lightPositionDirty = false;
    }

    // 临时测试的
    static bool runOnce = grubData( );
    Q_UNUSED( runOnce );

    foreach ( QObject* object, m_data )
    {
        Cube* cube = qobject_cast<Cube*>( object );
        Plane* plane = qobject_cast<Plane*>( object );
        TexturedCube* texturedCube = qobject_cast<TexturedCube*>( object );

        if ( cube != Q_NULLPTR ) cube->sync( );
        else if ( plane != Q_NULLPTR ) plane->sync( );
        else if ( texturedCube != Q_NULLPTR ) texturedCube->sync( );
    }
}

void View::cleanup( void )
{
    foreach ( QObject* object, m_data )
    {
        Cube* cube = qobject_cast<Cube*>( object );
        Plane* plane = qobject_cast<Plane*>( object );
        TexturedCube* texturedCube = qobject_cast<TexturedCube*>( object );

        if ( cube != Q_NULLPTR ) cube->release( );
        else if ( plane != Q_NULLPTR ) plane->release( );
        else if ( texturedCube != Q_NULLPTR ) texturedCube->release( );
    }

    delete m_FBO;
    delete m_depthProgram;
}

void View::renderShadow( void )
{
    // 根据各自的方法进行渲染阴影
    m_FBO->bind( );
    QOpenGLFunctions* f = window( )->openglContext( )->functions( );
    f->glViewport( 0.0,
                   0.0,
                   FBO_WIDTH,
                   FBO_HEIGHT );
    f->glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
    f->glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    f->glCullFace( GL_FRONT );
    m_depthProgram->bind( );
    m_depthProgram->setUniformValue( "viewProjectionMatrix", m_lightViewProjectionMatrix );
    foreach ( QObject* object, m_data )
    {
        Cube* cube = qobject_cast<Cube*>( object );
        Plane* plane = qobject_cast<Plane*>( object );

        if ( cube != Q_NULLPTR ) cube->renderShadow( );
        else if ( plane != Q_NULLPTR ) plane->renderShadow( );
    }
    m_depthProgram->release( );
    f->glCullFace( GL_BACK );

    m_FBO->bindDefault( );
}

void View::updateWindow( void )
{
    if ( window( ) != Q_NULLPTR ) window( )->update( );
}

void View::setPosition( const QVector3D& position )
{
    if ( m_position == position ) return;
    m_position = position;
    emit positionChanged( );
    calculateViewMatrix( );
}

void View::setLookAt( const QVector3D& lookAt )
{
    if ( m_lookAt == lookAt ) return;
    m_lookAt = lookAt;
    emit lookAtChanged( );
    calculateViewMatrix( );
}

void View::setUp( const QVector3D& up )
{
    if ( m_up == up ) return;
    m_up = up;
    emit upChanged( );
    calculateViewMatrix( );
}

void View::setFieldOfView( qreal fieldOfView )
{
    if ( m_fieldOfView == fieldOfView ) return;
    m_fieldOfView = fieldOfView;
    emit fieldOfViewChanged( );
    calculateProjectionMatrix( );
}

void View::setAspectRatio( qreal aspectRatio )
{
    if ( m_aspectRatio == aspectRatio ) return;
    m_aspectRatio = aspectRatio;
    emit aspectRatioChanged( );
    calculateProjectionMatrix( );
}

void View::setNearPlane( qreal nearPlane )
{
    if ( m_nearPlane == nearPlane ) return;
    m_nearPlane = nearPlane;
    emit nearPlaneChanged( );
    calculateProjectionMatrix( );
}

void View::setFarPlane( qreal farPlane )
{
    if ( m_farPlane == farPlane ) return;
    m_farPlane = farPlane;
    emit farPlaneChanged( );
    calculateProjectionMatrix( );
}

void View::setLightPosition( const QVector3D& lightPosition )
{
    if ( m_lightPosition == lightPosition ) return;
    m_lightPosition = lightPosition;
    emit lightPositionChanged( );
    m_lightPositionDirty = true;
    updateWindow( );
}

int View::shadowTexture( void )
{
    return m_FBO->texture( );
}

QQmlListProperty<QObject> View::data( void )
{
    return QQmlListProperty<QObject>( this,
                                      &m_data,
                                      qobjectListAppend,
                                      Q_NULLPTR,
                                      Q_NULLPTR,
                                      Q_NULLPTR );
}

void View::initialize( void )
{
    // 创建着色器
    m_depthProgram = new QOpenGLShaderProgram;
    m_depthProgram->addShaderFromSourceFile( QOpenGLShader::Vertex,
                                             ":/Depth.vert" );
    m_depthProgram->addShaderFromSourceFile( QOpenGLShader::Fragment,
                                             ":/Depth.frag" );
    m_depthProgram->link( );

    // 首先创建FBO
    m_FBO = new QOpenGLFramebufferObject( QSize( FBO_WIDTH, FBO_HEIGHT ) );

    foreach ( QObject* object, m_data )
    {
        Cube* cube = qobject_cast<Cube*>( object );
        Plane* plane = qobject_cast<Plane*>( object );
        TexturedCube* texturedCube = qobject_cast<TexturedCube*>( object );

        if ( cube != Q_NULLPTR ) cube->initialize( );
        else if ( plane != Q_NULLPTR ) plane->initialize( );
        else if ( texturedCube != Q_NULLPTR ) texturedCube->initialize( );
    }

    m_aspectRatio = float( window( )->width( ) ) /
            float( window( )->height( ) );
    calculateViewMatrix( );
    calculateProjectionMatrix( );
    connect( window( ), SIGNAL( beforeRendering( ) ),
             this, SLOT( render( ) ),
             Qt::DirectConnection );

    m_initialized = true;
}

void View::calculateViewMatrix( void )
{
    m_pendingViewMatrix.setToIdentity( );
    m_pendingViewMatrix.lookAt( m_position, m_lookAt, m_up );
    m_viewMatrixDirty = true;
    updateWindow( );
}

void View::calculateProjectionMatrix( void )
{
    m_pendingProjectionMatrix.setToIdentity( );
    m_pendingProjectionMatrix.perspective(
                m_fieldOfView, m_aspectRatio,
                m_nearPlane, m_farPlane );
    m_projectionMatrixDirty = true;
    updateWindow( );
}

void View::qobjectListAppend(
        QQmlListProperty<QObject>* prop, QObject* object )
{
    View* _this = qobject_cast<View*>( prop->object );
    qDebug( "the entity \"%s\" has been appended to \"%s\".",
            qPrintable( object->objectName( ) ),
            qPrintable( _this->objectName( ) ) );

    Cube* cube = qobject_cast<Cube*>( object );
    Plane* plane = qobject_cast<Plane*>( object );
    TexturedCube* texturedCube = qobject_cast<TexturedCube*>( object );

    if ( cube != Q_NULLPTR )
    {
        cube->setParent( _this );
        cube->setView( _this );
    }
    else if ( plane != Q_NULLPTR )
    {
        plane->setParent( _this );
        plane->setView( _this );
    }
    else if ( texturedCube != Q_NULLPTR )
    {
        texturedCube->setParent( _this );
        texturedCube->setView( _this );
    }

    reinterpret_cast<QObjectList*>( prop->data )->append( object );
}

#include <QDebug>
bool View::grubData( void )// 截取数据的
{
//    qDebug( ) << "plane's model matrix: ";
//    qDebug( ) << m_modelMatrix;

    qDebug( ) << "camera view matrix: ";
    qDebug( ) << m_viewMatrix;

    qDebug( ) << "camera projection matrix: ";
    qDebug( ) << m_projectionMatrix;

    qDebug( ) << "light position: ";
    qDebug( ) << m_lightPosition;

    qDebug( ) << "light view matrix: ";
    qDebug( ) << m_lightViewProjectionMatrix;

    qDebug( ) << "light projection matrix: ";
    qDebug( ) << m_projectionMatrix;

    return true;
}
