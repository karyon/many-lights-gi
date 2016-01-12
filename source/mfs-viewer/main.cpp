
#include <iostream>

#include <gloperate/ext-includes-begin.h>
#include <QApplication>
#include <QDesktopWidget>
#include <gloperate/ext-includes-end.h>

#include <globjects/base/baselogging.h>

#include <gloperate/plugin/PluginManager.h>
#include <gloperate/plugin/PainterPlugin.h>
#include <gloperate/resources/ResourceManager.h>
#include <gloperate-qt/viewer/QtTextureLoader.h>

#include "Viewer.h"

using namespace gloperate;
using namespace gloperate_qt;


int main(int argc, char * argv[])
{
    QApplication app(argc, argv);

    ResourceManager resourceManager;
    resourceManager.addLoader(new QtTextureLoader());

    PluginManager pluginManager;
    pluginManager.addSearchPath(QCoreApplication::applicationDirPath().toStdString());
    pluginManager.addSearchPath(QCoreApplication::applicationDirPath().toStdString() + "/plugins");

    #ifdef __APPLE__
        pluginManager.addSearchPath(QCoreApplication::applicationDirPath().toStdString() + "/../../..");
    #endif

    #ifdef NDEBUG
        pluginManager.scan("painters");
    #else
        pluginManager.scan("paintersd");
    #endif

    // Choose a painter
    #ifdef __APPLE__
        std::string name = "MultiFramePainter";
    #else
        std::string name = (argc > 1) ? argv[1] : "MultiFramePainter";
    #endif

    Plugin * plugin = pluginManager.plugin(name);
    if (!plugin)
    {
        globjects::fatal() << "Plugin '" << name << "' not found. Listing plugins found:";
        pluginManager.printPlugins();

        return 1;
    }

    AbstractPainterPlugin * painterPlugin = dynamic_cast<AbstractPainterPlugin *>(plugin);
    if (!painterPlugin)
    {
        globjects::fatal() << "Plugin '" << name << "' is not a painter plugin.";

        return 1;
    }

    gloperate::Painter* painter = painterPlugin->createPainter(resourceManager);

    Viewer viewer;
    viewer.show();
    viewer.setPainter(*painter);

    return app.exec();
}
