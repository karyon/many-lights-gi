
#include "Viewer.h"

#include <cassert>
#include <iomanip>

#include <gloperate/ext-includes-begin.h>

#include <QDockWidget>
#include <QDebug>
#include <QSettings>
#include <QStatusBar>

#include <propertyguizeug/PropertyBrowser.h>
#include <widgetzeug/MessageHandler.h>
#include <widgetzeug/MessageStatusWidget.h>
#include <widgetzeug/MessageWidget.h>

#include <gloperate/ext-includes-end.h>

#include <gloperate/resources/ResourceManager.h>

#include <gloperate-qt/viewer/QtOpenGLWindow.h>
#include <gloperate-qt/viewer/QtTextureLoader.h>
#include <gloperate-qt/viewer/QtKeyEventProvider.h>
#include <gloperate-qt/viewer/QtMouseEventProvider.h>
#include <gloperate-qt/viewer/QtWheelEventProvider.h>

#include <multiframepainter/MultiFramePainter.h>
#include "multiframepainter/PerfCounter.h"

#include "QtViewerMapping.h"

using namespace widgetzeug;
using namespace gloperate;


namespace
{
    const QString SETTINGS_GEOMETRY("Geometry");
    const QString SETTINGS_STATE("State");
    const QString SETTINGS_PLUGINS("Plugins");
}

Viewer::Viewer(QWidget * parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_resourceManager(nullptr)
    , m_painter(nullptr)
    , m_mapping(nullptr)
    , m_canvas(nullptr)
    , m_messagesStatus{ new MessageStatusWidget() }
    , m_messagesLog{ new MessageWidget() }
    , m_messagLogDockWidget(nullptr)
    , m_propertyDockWidget(nullptr)
{
    // Initialize resource manager (must be done BEFORE setupCanvas)
    m_resourceManager.reset(new ResourceManager());
    resize(1000, 500);

    // Add default texture loaders/storers
    m_resourceManager->addLoader(new gloperate_qt::QtTextureLoader());

    resize(1000, 500);
    // Setup UI
    setupMessageWidgets();
    resize(1000, 500);
    setupPropertyWidget();
    resize(1000, 500);
    setupCanvas();
    resize(1000, 500);

    // Load settings
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings settings;

    // Restore GUI state from settings
    //restoreGeometry(settings.value(SETTINGS_GEOMETRY).toByteArray());
    restoreState(settings.value(SETTINGS_STATE).toByteArray());

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer.start(20);
}

Viewer::~Viewer()
{
    m_canvas->makeCurrent();
    deinitializePainter();
    m_canvas->doneCurrent();

    // Save settings
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
    settings.setValue(SETTINGS_STATE, saveState());

    // Disconnect message handlers
    MessageHandler::dettach(*m_messagesLog);
    MessageHandler::dettach(*m_messagesStatus);
}

gloperate_qt::QtOpenGLWindow * Viewer::canvas() const
{
    return m_canvas.get();
}

void Viewer::setPainter(Painter & painter)
{
    // Unload old painter
    m_canvas->makeCurrent();
    deinitializePainter();
    m_canvas->doneCurrent();

    // Create new painter
    m_painter.reset(&painter);

    // [TODO] Check for painter context format requirements

    // Setup new painter
    m_canvas->setPainter(m_painter.get());
    m_mapping->setPainter(m_painter.get());
    m_canvas->initialize();

    // Update property browser
    if (m_painter.get())
    {
        QWidget * old = m_propertyDockWidget->widget();
        delete old;

        auto propertyBrowser = new propertyguizeug::PropertyBrowser(m_painter.get());
        m_propertyDockWidget->setWidget(propertyBrowser);
        m_propertyDockWidget->show();
        propertyBrowser->setAlwaysExpandGroups(true);
    }
    else
    {
        m_propertyDockWidget->hide();
    }

    // Update rendering
    m_canvas->updateGL();

    m_canvas.get()->makeCurrent();
    m_canvas.get()->doneCurrent();
}

void Viewer::setupMessageWidgets()
{
    // Widgets have to be created beforehand
    assert(m_messagesStatus);
    assert(m_messagesLog);

    // Attach message handlers to log widgets
    MessageHandler::attach(*m_messagesStatus);
    MessageHandler::attach(*m_messagesLog);

    // Additionally write log to file
    MessageHandler::printsToFile(QtMsgType::QtWarningMsg);
    MessageHandler::printsToFile(QtMsgType::QtCriticalMsg);
    MessageHandler::printsToFile(QtMsgType::QtFatalMsg);

    // Announce log initialization
    qDebug("Initialize message log.");
    const QString fileLog(QString("Messages are also written to file://%1.").arg(MessageHandler::fileName()));
    qDebug("%s", qPrintable(fileLog));

    m_infoDockWidget = new QDockWidget(tr("Info"));
    m_infoDockWidget->setWidget(&m_infoLabel);
    addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, m_infoDockWidget);

    // Create dock window for message log
    m_messagLogDockWidget = new QDockWidget(tr("Message Log"));
    m_messagLogDockWidget->setWidget(m_messagesLog.get());
    m_messagLogDockWidget->setObjectName("MessageLogDockWidget");
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_messagLogDockWidget);
    m_messagesLog->setFrameShape(QFrame::NoFrame);

    // Add item to status bar and connect to window
    statusBar()->addPermanentWidget(m_messagesStatus.get());
    m_messagesStatus->attachWidget(m_messagLogDockWidget);
}

void Viewer::setupPropertyWidget()
{
    // Create dock window for property browser
    m_propertyDockWidget = new QDockWidget("Properties", this);
    m_propertyDockWidget->setObjectName("PropertyDockWidget");
    m_propertyDockWidget->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, m_propertyDockWidget, Qt::Orientation::Horizontal);
}

void Viewer::setupCanvas()
{
    QSurfaceFormat format;

    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(16);

    // Create OpenGL context and window
    m_canvas.reset(new gloperate_qt::QtOpenGLWindow(*m_resourceManager, format));

    // Create widget container
    setCentralWidget(QWidget::createWindowContainer(m_canvas.get()));
    centralWidget()->setFocusPolicy(Qt::StrongFocus);

    // Setup event provider to translate Qt messages into gloperate events
    gloperate_qt::QtKeyEventProvider * keyProvider = new gloperate_qt::QtKeyEventProvider();
    keyProvider->setParent(m_canvas.get());
    gloperate_qt::QtMouseEventProvider * mouseProvider = new gloperate_qt::QtMouseEventProvider();
    mouseProvider->setParent(m_canvas.get());
    gloperate_qt::QtWheelEventProvider * wheelProvider = new gloperate_qt::QtWheelEventProvider();
    wheelProvider->setParent(m_canvas.get());

    m_canvas->installEventFilter(keyProvider);
    m_canvas->installEventFilter(mouseProvider);
    m_canvas->installEventFilter(wheelProvider);

    // Create input mapping for gloperate interaction techniques
    m_mapping.reset(new QtViewerMapping(m_canvas.get()));
    m_mapping->addProvider(keyProvider);
    m_mapping->addProvider(mouseProvider);
    m_mapping->addProvider(wheelProvider);
}

void Viewer::deinitializePainter()
{
    m_painter.reset(nullptr);
}

void Viewer::onTimer()
{
    m_infoLabel.setText("");
    auto mfPainter = dynamic_cast<MultiFramePainter*>(m_painter.get());
    if (mfPainter)
    {
        auto str = mfPainter->getPerfCounterString();
        m_infoLabel.setText(QString::fromStdString(str));
    }
	((propertyguizeug::PropertyBrowser*)m_propertyDockWidget->widget())->expandAllGroups();
}

gloperate::ResourceManager * Viewer::resourceManager()
{
    return m_resourceManager.get();
}
