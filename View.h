#ifndef VIEW_H
#define VIEW_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuickItem>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
class QOpenGLFramebufferObject;
QT_END_NAMESPACE

class View: public QQuickItem
{
    Q_OBJECT

    // 相机属性
    Q_PROPERTY( QVector3D position READ position WRITE setPosition NOTIFY positionChanged )
    Q_PROPERTY( QVector3D lookAt READ lookAt WRITE setLookAt NOTIFY lookAtChanged )
    Q_PROPERTY( QVector3D up READ up WRITE setUp NOTIFY upChanged )
    Q_PROPERTY( qreal fieldOfView READ fieldOfView WRITE setFieldOfView NOTIFY fieldOfViewChanged )
    Q_PROPERTY( qreal aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged )
    Q_PROPERTY( qreal nearPlane READ nearPlane WRITE setNearPlane NOTIFY nearPlaneChanged )
    Q_PROPERTY( qreal farPlane READ farPlane WRITE setFarPlane NOTIFY farPlaneChanged )

    // 光源属性
    Q_PROPERTY( QVector3D lightPosition READ lightPosition
                WRITE setLightPosition NOTIFY lightPositionChanged )
//    Q_PROPERTY( QVector3D lightLookAt READ lightLookAt WRITE setLightLookAt NOTIFY lightLookAtChanged )
//    Q_PROPERTY( QVector3D lightUp READ lightUp WRITE setLightUp NOTIFY lightUpChanged )

    // 支持默认孩子
    Q_PROPERTY( QQmlListProperty<QObject> data READ data )
    Q_CLASSINFO( "DefaultProperty", "data" )
public:
    View( QQuickItem* parent = Q_NULLPTR );
    ~View( void );

    QVector3D position( void ) { return m_position; }
    void setPosition( const QVector3D& position );

    QVector3D lookAt( void ) { return m_lookAt; }
    void setLookAt( const QVector3D& lookAt );

    QVector3D up( void ) { return m_up; }
    void setUp( const QVector3D& up );

    qreal fieldOfView( void ) { return m_fieldOfView; }
    void setFieldOfView( qreal fieldOfView );

    qreal aspectRatio( void ) { return m_aspectRatio; }
    void setAspectRatio( qreal aspectRatio );

    qreal nearPlane( void ) { return m_nearPlane; }
    void setNearPlane( qreal nearPlane );

    qreal farPlane( void ) { return m_farPlane; }
    void setFarPlane( qreal farPlane );

    QVector3D lightPosition( void ) { return m_lightPosition; }
    void setLightPosition( const QVector3D& lightPosition );

    QMatrix4x4& viewMatrix( void ) { return m_viewMatrix; }
    QMatrix4x4& projectionMatrix( void ) { return m_projectionMatrix; }
    QMatrix4x4& lightViewProjectionMatrix( void ) { return m_lightViewProjectionMatrix; }
    int shadowTexture( void );
    QOpenGLShaderProgram* depthProgram( void ) { return m_depthProgram; }

    QQmlListProperty<QObject> data( void );
    void initialize( void );
signals:
    void positionChanged( void );
    void lookAtChanged( void );
    void upChanged( void );
    void aspectRatioChanged( void );
    void fieldOfViewChanged( void );
    void nearPlaneChanged( void );
    void farPlaneChanged( void );
    void propertyChanged( void );
    void lightPositionChanged( void );
protected slots:
    void onWindowChanged( QQuickWindow* win );
    void render( void );
    void sync( void );
    void cleanup( void );
protected:
    void renderShadow( void );
    void updateWindow( void );
    void calculateViewMatrix( void );
    void calculateProjectionMatrix( void );
    static void qobjectListAppend( QQmlListProperty<QObject>* prop, QObject* object );

    // 临时
    bool grubData( void );

    QObjectList                 m_data;

    QVector3D                   m_position, m_lookAt, m_up;
    qreal                       m_aspectRatio, m_fieldOfView;
    qreal                       m_nearPlane, m_farPlane;
    QVector3D                   m_lightPosition;

    // 视角矩阵以及投影矩阵
    QMatrix4x4                  m_viewMatrix;
    QMatrix4x4                  m_projectionMatrix;

    // 防止访问错误，使用临时的
    QMatrix4x4                  m_pendingViewMatrix;
    QMatrix4x4                  m_pendingProjectionMatrix;
    bool                        m_viewMatrixDirty: 1;
    bool                        m_projectionMatrixDirty: 1;

    // 渲染阴影用的
    bool                        m_lightPositionDirty: 1;
    QMatrix4x4                  m_lightViewProjectionMatrix;
    QOpenGLShaderProgram*       m_depthProgram;
    QOpenGLFramebufferObject*   m_FBO;

    bool                        m_initialized;
};

#endif // VIEW_H
