#ifndef MYPLANE_H
#define MYPLANE_H

#include <QUrl>
#include <QVector3D>
#include <QObject>

class View;
class PlaneRenderer;
class Plane: public QObject
{
    Q_OBJECT
    Q_PROPERTY( qreal length READ length WRITE setLength NOTIFY lengthChanged )
    Q_PROPERTY( QUrl source READ source WRITE setSource NOTIFY sourceChanged )
    Q_PROPERTY( QVector3D translate READ translate WRITE setTranslate NOTIFY translateChanged )
public:
    explicit Plane( QObject* parent = Q_NULLPTR );

    void initialize( void );
    void render( void );
    void renderShadow( void );
    void sync( void );
    void release( void );
    void setView( View* view ) { m_view = view; }

    qreal length( void ) { return m_length; }
    void setLength( qreal length );

    QUrl source( void ) { return m_source; }
    void setSource( const QUrl& source );

    QVector3D translate( void ) { return m_translate; }
    void setTranslate( const QVector3D& translate );

    friend class PlaneRenderer;
signals:
    void lengthChanged( void );
    void sourceChanged( void );
    void translateChanged( void );
protected:
    qreal           m_length;
    QUrl            m_source;
    QVector3D       m_translate;

    bool            m_lengthIsDirty: 1;
    bool            m_sourceIsDirty: 1;
    bool            m_translateIsDirty: 1;

    View* m_view;
    PlaneRenderer*  m_renderer;
};


#endif // MYPLANE_H
