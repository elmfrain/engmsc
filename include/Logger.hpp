#ifndef EM_LOGGER_HPP
#define EM_LOGGER_HPP

#include <string>

class EMLogger
{
public:
    EMLogger();
    EMLogger(const char* moduleName);

    const std::string& getModuleName() const;

    EMLogger& submodule(const char* submodule);

    EMLogger& infof(const char* fmt, ...);
    EMLogger& warnf(const char* fmt, ...);
    EMLogger& errorf(const char* fmt, ...);
    EMLogger& fatalf(const char* fmt, ...);
private:
    std::string m_moduleName;
    std::string m_submoduleName;
};

#endif // EM_LOGGER_HPP