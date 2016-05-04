#pragma once

#include <cstdint>
#include <string>

class PerfCounter
{
public:
    static void begin(const std::string & name);
    static void beginGL(const std::string & name);
    static void end(const std::string & name);
    static void endGL(const std::string & name);
    static std::string generateString();

protected:
    static void addNameToOrderedNames(const std::string & name);
    static void addMeasurement(const std::string & name, uint64_t nanoseconds);
};

class AutoGLPerfCounter
{
public:
    AutoGLPerfCounter(std::string name);
    ~AutoGLPerfCounter();
protected:
    std::string m_name;
};

class AutoPerfCounter
{
public:
    AutoPerfCounter(std::string name);
    ~AutoPerfCounter();
protected:
    std::string m_name;
};