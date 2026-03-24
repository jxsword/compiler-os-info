#include <stdio.h>
#include <string.h>

/* 编译器检测宏 */
#ifdef __clang__
#define COMPILER_NAME "Clang"
#define COMPILER_VERSION __clang_version__
#elif defined(__GNUC__)
#define COMPILER_NAME "GCC"
#define COMPILER_VERSION __VERSION__
#elif defined(_MSC_VER)
#define COMPILER_NAME "MSVC"
#define COMPILER_VERSION_STRING(x) #x
#define COMPILER_VERSION(x) COMPILER_VERSION_STRING(x)
#define COMPILER_VERSION_VALUE COMPILER_VERSION(_MSC_VER)
#else
#define COMPILER_NAME "Unknown Compiler"
#define COMPILER_VERSION "Unknown Version"
#endif

/* 操作系统检测和API调用 */
#ifdef _WIN32
#include <windows.h>
#define OS_NAME "Windows"

/* 替代RtlGetVersion函数，可以获取真实版本 */
typedef struct _RTL_OSVERSIONINFOEXW_CUSTOM
{
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR szCSDVersion[128];
    USHORT wServicePackMajor;
    USHORT wServicePackMinor;
    USHORT wSuiteMask;
    UCHAR wProductType;
    UCHAR wReserved;
} RTL_OSVERSIONINFOEXW_CUSTOM, *PRTL_OSVERSIONINFOEXW_CUSTOM;

typedef LONG(WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOEXW_CUSTOM);

void get_os_info(char *buffer, size_t buffer_size)
{
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    RtlGetVersionPtr pRtlGetVersion = NULL;

    if (hNtdll)
    {
        pRtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
    }

    if (pRtlGetVersion)
    {
        RTL_OSVERSIONINFOEXW_CUSTOM osvi = {0};
        osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW_CUSTOM);

        if (pRtlGetVersion(&osvi) == 0)
        { // STATUS_SUCCESS
            if (osvi.dwMajorVersion == 10)
            {
                if (osvi.dwBuildNumber >= 22000)
                {
                    snprintf(buffer, buffer_size, "Windows 11 (Build %lu)", osvi.dwBuildNumber);
                }
                else
                {
                    snprintf(buffer, buffer_size, "Windows 10 (Build %lu)", osvi.dwBuildNumber);
                }
            }
            else if (osvi.dwMajorVersion == 6)
            {
                if (osvi.dwMinorVersion == 3)
                {
                    snprintf(buffer, buffer_size, "Windows 8.1 (Build %lu)", osvi.dwBuildNumber);
                }
                else if (osvi.dwMinorVersion == 2)
                {
                    snprintf(buffer, buffer_size, "Windows 8 (Build %lu)", osvi.dwBuildNumber);
                }
                else if (osvi.dwMinorVersion == 1)
                {
                    snprintf(buffer, buffer_size, "Windows 7 (Build %lu)", osvi.dwBuildNumber);
                }
                else if (osvi.dwMinorVersion == 0)
                {
                    snprintf(buffer, buffer_size, "Windows Vista (Build %lu)", osvi.dwBuildNumber);
                }
                else
                {
                    snprintf(buffer, buffer_size, "Windows NT 6.%lu (Build %lu)",
                             osvi.dwMinorVersion, osvi.dwBuildNumber);
                }
            }
            else if (osvi.dwMajorVersion == 5)
            {
                if (osvi.dwMinorVersion == 2)
                {
                    snprintf(buffer, buffer_size, "Windows Server 2003/XP x64 (Build %lu)", osvi.dwBuildNumber);
                }
                else if (osvi.dwMinorVersion == 1)
                {
                    snprintf(buffer, buffer_size, "Windows XP (Build %lu)", osvi.dwBuildNumber);
                }
                else if (osvi.dwMinorVersion == 0)
                {
                    snprintf(buffer, buffer_size, "Windows 2000 (Build %lu)", osvi.dwBuildNumber);
                }
                else
                {
                    snprintf(buffer, buffer_size, "Windows NT 5.%lu (Build %lu)",
                             osvi.dwMinorVersion, osvi.dwBuildNumber);
                }
            }
            else
            {
                snprintf(buffer, buffer_size, "Windows NT %lu.%lu (Build %lu)",
                         osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
            }
        }
        else
        {
            /* 回退到GetVersionExW */
            OSVERSIONINFOEXW osvi_fallback = {0};
            osvi_fallback.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

            if (GetVersionExW((OSVERSIONINFOW *)&osvi_fallback))
            {
                if (osvi_fallback.dwMajorVersion == 10)
                {
                    snprintf(buffer, buffer_size, "Windows 10/11 (Build %lu)", osvi_fallback.dwBuildNumber);
                }
                else
                {
                    snprintf(buffer, buffer_size, "Windows %lu.%lu (Build %lu)",
                             osvi_fallback.dwMajorVersion, osvi_fallback.dwMinorVersion,
                             osvi_fallback.dwBuildNumber);
                }
            }
            else
            {
                snprintf(buffer, buffer_size, "Windows (Unknown Version)");
            }
        }
    }
    else
    {
        /* 如果没有RtlGetVersion，使用GetVersionExW */
        OSVERSIONINFOEXW osvi_fallback = {0};
        osvi_fallback.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

        if (GetVersionExW((OSVERSIONINFOW *)&osvi_fallback))
        {
            snprintf(buffer, buffer_size, "Windows %lu.%lu (Build %lu)",
                     osvi_fallback.dwMajorVersion, osvi_fallback.dwMinorVersion,
                     osvi_fallback.dwBuildNumber);
        }
        else
        {
            snprintf(buffer, buffer_size, "Windows (Unknown Version)");
        }
    }
}

#elif defined(__APPLE__) && defined(__MACH__)
#include <sys/utsname.h>
#include <stdlib.h>
#define OS_NAME "macOS"

void get_os_info(char *buffer, size_t buffer_size)
{
    struct utsname uname_data;
    if (uname(&uname_data) == 0)
    {
/* 尝试获取更详细的macOS版本信息 */
#ifdef __APPLE__
        FILE *fp = popen("sw_vers -productVersion 2>/dev/null", "r");
        if (fp)
        {
            char version[64] = {0};
            if (fgets(version, sizeof(version), fp))
            {
                version[strcspn(version, "\n")] = 0;
                if (strlen(version) > 0)
                {
                    snprintf(buffer, buffer_size, "macOS %s (Kernel: %s)",
                             version, uname_data.release);
                }
                else
                {
                    snprintf(buffer, buffer_size, "macOS (Kernel: %s)",
                             uname_data.release);
                }
            }
            else
            {
                snprintf(buffer, buffer_size, "macOS (Kernel: %s)",
                         uname_data.release);
            }
            pclose(fp);
        }
        else
        {
            snprintf(buffer, buffer_size, "macOS (Kernel: %s)",
                     uname_data.release);
        }
#else
        snprintf(buffer, buffer_size, "Darwin (Kernel: %s)", uname_data.release);
#endif
    }
    else
    {
        snprintf(buffer, buffer_size, "macOS (Unknown Version)");
    }
}

#elif defined(__linux__)
#include <sys/utsname.h>
#define OS_NAME "Linux"

void get_os_info(char *buffer, size_t buffer_size)
{
    struct utsname uname_data;
    if (uname(&uname_data) == 0)
    {
        /* 尝试检测是否是Ubuntu */
        FILE *fp = fopen("/etc/os-release", "r");
        if (fp)
        {
            char line[256];
            int found = 0;

            while (fgets(line, sizeof(line), fp))
            {
                if (strncmp(line, "PRETTY_NAME=", 12) == 0)
                {
                    char *start = line + 12;
                    char *end = strchr(start, '\n');
                    if (end)
                        *end = '\0';

                    /* 移除引号 */
                    if (start[0] == '"' || start[0] == '\'')
                    {
                        start++;
                    }
                    size_t len = strlen(start);
                    if (len > 0 && (start[len - 1] == '"' || start[len - 1] == '\''))
                    {
                        start[len - 1] = '\0';
                    }

                    snprintf(buffer, buffer_size, "%s", start);
                    found = 1;
                    break;
                }
            }
            fclose(fp);

            if (!found)
            {
                snprintf(buffer, buffer_size, "Linux %s %s",
                         uname_data.release, uname_data.machine);
            }
        }
        else
        {
            /* 如果无法读取os-release，则使用uname信息 */
            snprintf(buffer, buffer_size, "Linux %s %s",
                     uname_data.release, uname_data.machine);
        }
    }
    else
    {
        snprintf(buffer, buffer_size, "Linux (Unknown Version)");
    }
}

#else
#define OS_NAME "Unknown OS"

void get_os_info(char *buffer, size_t buffer_size)
{
    snprintf(buffer, buffer_size, "Unknown Operating System");
}
#endif

int main()
{
    char os_info[256] = {0};

    /* 获取操作系统信息 */
    get_os_info(os_info, sizeof(os_info));

    /* 输出信息 */
    printf("========================================\n");
    printf("Compiler and OS Information\n");
    printf("========================================\n");

    /* 编译器信息 */
    printf("Compiler: %s\n", COMPILER_NAME);

#ifdef _MSC_VER
    printf("Compiler Version: MSVC %s\n", COMPILER_VERSION_VALUE);
#elif defined(__GNUC__) || defined(__clang__)
    printf("Compiler Version: %s\n", COMPILER_VERSION);
#else
    printf("Compiler Version: Unknown\n");
#endif

    /* 操作系统信息 */
    printf("Operating System: %s\n", os_info);

/* 架构信息 */
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
    printf("Architecture: 64-bit\n");
#elif defined(_WIN32) && !defined(_WIN64)
    printf("Architecture: 32-bit\n");
#else
    printf("Architecture: Unknown\n");
#endif

    printf("========================================\n");

    return 0;
}