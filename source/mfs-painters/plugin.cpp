
#include <multiframesampling/multiframesampling-version.h>

#include <gloperate/plugin/plugin_api.h>

#include "multiframepainter/MultiFramePainter.h"
#include "singleframepainter/SingleFramePainter.h"


GLOPERATE_PLUGIN_LIBRARY

    GLOPERATE_PAINTER_PLUGIN(MultiFramePainter
    , "MultiFramePainter"
    , "Rendering using multi-frame sampling."
    , MULTIFRAMESAMPLING_AUTHOR_ORGANIZATION
    , "v1.0.0" )

    GLOPERATE_PAINTER_PLUGIN(SingleFramePainter
    , "SingleFramePainter"
    , "Rendering using single complete frames."
    , MULTIFRAMESAMPLING_AUTHOR_ORGANIZATION
    , "v1.0.0" )

GLOPERATE_PLUGIN_LIBRARY_END
