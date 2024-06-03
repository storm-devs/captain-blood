#ifndef __AG_WIN32_REGISTRY_ACCESS__
#define __AG_WIN32_REGISTRY_ACCESS__

/* AgWin32RegistryAccess is not part of the API and should not be included in the documentation.*/
/** \cond */

#include <windows.h>

#define AG_BASE_KEY_NAME            "Software\\AGEIA Technologies\\"
#define AG_LOCAL_DLL_SUBKEY_NAME    "enableLocalPhysXCore"
#define AG_NO_NIC_MAC               "AGEIA"

class AgWin32RegistryAccess
{
public:
    static bool ReadRegKey(const char *inKey, const char *inSubKey, char *outData, unsigned int *length);
    static bool WriteRegKey(const char *inKey, const char *inSubKey, const char *inData, unsigned int length, bool nonvolatile, unsigned int wordType=REG_SZ);
    bool EnableLocalDllUsage();
    bool DisableLocalDllUsage();
    bool IsLocalDllUsageEnabled();

private:
    bool getMACAddr(unsigned char addr[8]);
};

/** \endcond */

#endif
