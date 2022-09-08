#include "Logger.hpp"

#include <stdarg.h>
#include <stdio.h>

EMLogger::EMLogger() : 
    m_moduleName("Logger")
{
}

EMLogger::EMLogger(const char* moduleName) :
    m_moduleName(moduleName)
{
}

const std::string& EMLogger::getModuleName() const
{
    return m_moduleName;
}

EMLogger& EMLogger::submodule(const char* submodule)
{
    m_submoduleName = submodule;

    return *this;
}

EMLogger& EMLogger::infof(const char* fmt, ...)
{
    char info[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(info, sizeof(info), fmt, args);
    va_end(args);

    if(!m_submoduleName.empty())
    {
        printf("[INFO][%s][%s]: %s\n", m_moduleName.c_str(), m_submoduleName.c_str(), info);
        m_submoduleName.clear();
    }
    else
    {
        printf("[INFO][%s]: %s\n", m_moduleName.c_str(), info);
    }

    return *this;
}

EMLogger& EMLogger::warnf(const char* fmt, ...)
{
    char info[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(info, sizeof(info), fmt, args);
    va_end(args);

    if(!m_submoduleName.empty())
    {
        printf("[WARNING][%s][%s]: %s\n", m_moduleName.c_str(), m_submoduleName.c_str(), info);
        m_submoduleName.clear();
    }
    else
    {
        printf("[WARNING][%s]: %s\n", m_moduleName.c_str(), info);
    }

    return *this;
}

EMLogger& EMLogger::errorf(const char* fmt, ...)
{
    char info[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(info, sizeof(info), fmt, args);
    va_end(args);

    if(!m_submoduleName.empty())
    {
        printf("[ERROR][%s][%s]: %s\n", m_moduleName.c_str(), m_submoduleName.c_str(), info);
        m_submoduleName.clear();
    }
    else
    {
        printf("[ERROR][%s]: %s\n", m_moduleName.c_str(), info);
    }

    return *this;
}

EMLogger& EMLogger::fatalf(const char* fmt, ...)
{
    char info[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(info, sizeof(info), fmt, args);
    va_end(args);

    if(!m_submoduleName.empty())
    {
        printf("[FATAL][%s][%s]: %s\n", m_moduleName.c_str(), m_submoduleName.c_str(), info);
        m_submoduleName.clear();
    }
    else
    {
        printf("[FATAL][%s]: %s\n", m_moduleName.c_str(), info);
    }

    return *this;
}