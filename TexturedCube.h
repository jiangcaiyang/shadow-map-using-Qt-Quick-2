#ifndef TEXTURECUBE_H
#define TEXTURECUBE_H

// 为了测试阴影映射，暂时不要使用这个类

#include <QVector3D>
#include <QUrl>
#include <QObject>

class View;
class TexturedCubeRenderer;
class TexturedCube: public QObject
{
    Q_OBJECT
    Q_PROPERTY( qreal length READ length WRITE setLength NOTIFY lengthChanged )
    Q_PROPERTY( QUrl source READ source WRITE setSource NOTIFY sourceChanged )
    Q_PROPERTY( QVector3D translate READ translate WRITE setTranslate NOTIFY translateChanged )
public:
    TexturedCube( QObject* parent = Q_NULLPTR );
    void initialize( void );
    void render( void );
    void sync( void );
    void release( void );
    void setView( View* view ) { m_view = view; }

    qreal length( void ) { return m_length; }
    void setLength( qreal length );

    QUrl source( void ) { return m_source; }
    void setSource( const QUrl& source );

    QVector3D translate( void ) { return m_translate; }
    void setTranslate( const QVector3D& translate );

    friend class TexturedCubeRenderer;
signals:
    void lengthChanged( void );
    void sourceChanged( void );
    void translateChanged( void );
protected:
    void updateWindow( void );

    qreal               m_length;
    bool                m_lengthDirty;

    QUrl                m_source;
    bool                m_sourceDirty;

    // 模型矩阵
    QVector3D           m_translate;

    TexturedCubeRenderer* m_renderer;
    View*   m_view;
};
#endif // TEXTURECUBE_H
