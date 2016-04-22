#pragma once

#include <memory>

#include <QMainWindow>
#include <QTimer>
#include <QLabel>


namespace widgetzeug
{
    class MessageStatusWidget;
    class MessageWidget;
}

namespace gloperate
{
    class ResourceManager;
    class Painter;
}

namespace gloperate_qt
{
    class QtOpenGLWindow;
    class ScriptEnvironment;
}

class QtViewerMapping;


class Viewer : public QMainWindow
{
    Q_OBJECT


public:
    Viewer(QWidget * parent = nullptr, Qt::WindowFlags flags = 0);
    virtual ~Viewer();

    gloperate_qt::QtOpenGLWindow * canvas() const;

    void setPainter(gloperate::Painter & painter);

    gloperate::ResourceManager * resourceManager();
protected:
    void setupMessageWidgets();
    void setupPropertyWidget();
    void setupCanvas();

    void deinitializePainter();

protected slots:
    void onTimer();

protected:
    std::unique_ptr<gloperate::ResourceManager>      m_resourceManager;

    std::unique_ptr<gloperate::Painter>              m_painter;
    std::unique_ptr<QtViewerMapping>                 m_mapping;

    std::unique_ptr<gloperate_qt::QtOpenGLWindow>    m_canvas;
    std::unique_ptr<widgetzeug::MessageStatusWidget> m_messagesStatus;
    std::unique_ptr<widgetzeug::MessageWidget>       m_messagesLog;

    QLabel                                           m_infoLabel;
    QTimer                                           m_timer;

    QDockWidget                                    * m_infoDockWidget;
    QDockWidget                                    * m_messagLogDockWidget;
    QDockWidget                                    * m_propertyDockWidget;
};
