#ifndef SHAHCOIN_UTIL_SYSTEM_H
#define SHAHCOIN_UTIL_SYSTEM_H

#include <string>
#include <vector>
#include <cstdint>

void SetupEnvironment();
void PrintExceptionContinue(const std::exception *pex, const char *pszThread);
bool TruncateFile(FILE *file, unsigned int length);
int RaiseFileDescriptorLimit(int nMinFD);

std::string GetDefaultDataDir();
std::string GetDataDir();
bool TryCreateDirectories(const std::string &path);
std::string GetConfigFile();

#endif // SHAHCOIN_UTIL_SYSTEM_H
