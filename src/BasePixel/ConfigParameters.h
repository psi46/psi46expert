// Configuration parameters

#ifndef CONFIGPARAMETERS
#define CONFIGPARAMETERS

#include <string>

class ConfigParameters
{
public:
    ConfigParameters();
    ConfigParameters(char * fileName);

    void Initialize();
    static ConfigParameters * Singleton();

    // == accessing =============================================================

    int nRocs, nModules, hubId, dataTriggerLevel, halfModule;
    int customModule;
    int emptyReadoutLength, emptyReadoutLengthADC, emptyReadoutLengthADCDual, tbmChannel;
    double ia, id, va, vd;
    float rocZeroAnalogCurrent;
    std::string roc_type;

    char directory[1000];
    char testboardName[1000];
    bool hvOn, tbmEnable, tbmEmulator, keithleyRemote, guiMode;

    const char * GetTbParametersFileName();
    const char * GetDacParametersFileName();
    const char * GetTbmParametersFileName();
    const char * GetTrimParametersFileName();
    const char * GetTestParametersFileName();
    const char * GetRootFileName();
    const char * GetLogFileName();
    const char * GetMaskFileName();

    const std::string GetDebugFileName();

    void SetTbParameterFileName(const std::string &filename);
    void SetDacParameterFileName(const std::string &filename);
    void SetTbmParameterFileName(const std::string &filename);
    void SetTrimParameterFileName(const std::string &filename);
    void SetTestParameterFileName(const std::string &filename);
    void SetRootFileName(const std::string &filename);
    void SetLogFileName(const std::string &filename);
    void SetDebugFileName(const std::string &filename);
    void SetMaskFileName(const std::string &filename);


    // == file input / output ===================================================

    bool ReadConfigParameterFile(const char * filename);
    bool WriteConfigParameterFile();

private:
    std::string tbParametersFileName;
    std::string dacParametersFileName;
    std::string tbmParametersFileName;
    std::string trimParametersFileName;
    std::string rootFileName;
    std::string logFileName;
    std::string debugFileName;
    std::string testParametersFileName;
    std::string maskFileName;

    static ConfigParameters * instance;
};

inline const char * ConfigParameters::GetTbParametersFileName() { return tbParametersFileName.c_str(); }
inline const char * ConfigParameters::GetDacParametersFileName() { return dacParametersFileName.c_str(); }
inline const char * ConfigParameters::GetTbmParametersFileName() { return tbmParametersFileName.c_str(); }
inline const char * ConfigParameters::GetTrimParametersFileName() { return trimParametersFileName.c_str(); }
inline const char * ConfigParameters::GetTestParametersFileName() { return testParametersFileName.c_str(); }
inline const char * ConfigParameters::GetRootFileName() { return rootFileName.c_str(); }
inline const char * ConfigParameters::GetLogFileName() { return logFileName.c_str(); }
inline const char * ConfigParameters::GetMaskFileName() { return maskFileName.c_str(); }

inline const std::string ConfigParameters::GetDebugFileName() { return debugFileName; }

#endif
