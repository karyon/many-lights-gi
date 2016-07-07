#include "PerfCounter.h"

#include <cassert>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Query.h>
#include <globjects/base/ref_ptr.h>
#include <gloperate/base/ChronoTimer.h>

#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>

using namespace gl;
using namespace globjects;

namespace
{
    static std::unordered_map<std::string, uint64_t> map;
    static std::unordered_map<std::string, gloperate::ChronoTimer> timerMap;
    static std::vector<std::string> orderedNames;
    static const float smoothingFactor = 0.95f;

    static std::unordered_map<std::string, ref_ptr<Query>> glTimerMap;
    static std::string runningGLQuery("");
}

void PerfCounter::begin(const std::string & name)
{
    assert(timerMap.find(name) != timerMap.end());
    timerMap[name] = gloperate::ChronoTimer();
}

void PerfCounter::end(const std::string & name)
{
    assert(timerMap.find(name) != timerMap.end());

    auto elapsedTime = timerMap[name].elapsed();
    timerMap.erase(name);

    addNameToOrderedNames(name);

    addMeasurement(name, elapsedTime.count());
}

void PerfCounter::beginGL(const std::string & name)
{
    assert(runningGLQuery == "");
    runningGLQuery = name;

    if (glTimerMap.find(name) == glTimerMap.end()) {
        auto query = new Query();
        glTimerMap[name] = query;
    }
    else {
        addMeasurement(name, glTimerMap[name]->get(GL_QUERY_RESULT));
    }

    glTimerMap[name]->begin(GL_TIME_ELAPSED);
}

void PerfCounter::endGL(const std::string & name)
{
    assert(glTimerMap.find(name) != glTimerMap.end());
    assert(runningGLQuery == name);

    addNameToOrderedNames(name);

    runningGLQuery = "";

    glTimerMap[name]->end(GL_TIME_ELAPSED);
}

std::string PerfCounter::generateString()
{
    std::stringstream ss;
    ss.precision(2);

    for (std::string name : orderedNames)
        ss << name << ": " << std::fixed << map[name] / 1000000.0 << "  ";
    return ss.str();
}

void PerfCounter::addNameToOrderedNames(const std::string & name)
{
    if (std::find(orderedNames.begin(), orderedNames.end(), name) == orderedNames.end())
        orderedNames.push_back(name);
}

void PerfCounter::addMeasurement(const std::string & name, uint64_t nanoseconds)
{
    if (map[name] == 0L)
        map[name] = nanoseconds;
    else {
        auto initial = map[name];
        auto i = 1 - smoothingFactor;
        auto a = map[name] * smoothingFactor;
        map[name] = uint64_t(nanoseconds * (1 - smoothingFactor) + map[name] * smoothingFactor);
    }
}


AutoGLPerfCounter::AutoGLPerfCounter(std::string name)
: m_name(name)
, m_debugGroup(name)
{
    PerfCounter::beginGL(m_name);
}

AutoGLPerfCounter::~AutoGLPerfCounter()
{
    PerfCounter::endGL(m_name);
}


AutoGLDebugGroup::AutoGLDebugGroup(std::string name)
: m_name(name)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name.data());
}

AutoGLDebugGroup::~AutoGLDebugGroup()
{
    glPopDebugGroup();
}


AutoPerfCounter::AutoPerfCounter(std::string name)
: m_name(name)
{
    PerfCounter::begin(m_name);
}

AutoPerfCounter::~AutoPerfCounter()
{
    PerfCounter::end(m_name);
}
