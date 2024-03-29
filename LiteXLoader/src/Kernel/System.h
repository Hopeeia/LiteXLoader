#include "Global.h"
#include <string>
#include <functional>

std::string Raw_GetDateTimeStr();
std::string Raw_RandomGuid();

bool Raw_DirCreate(const std::string &path);
bool Raw_PathExists(const std::string &path);
bool Raw_FileReadAll(const std::string &path, std::string & result);
bool Raw_FileWriteAll(const std::string &path, const std::string &data);

bool Raw_SystemCmd(const std::string &cmd, std::function<void(int,std::string)> callback, int timeLimit);

std::pair<int,std::string> Raw_HttpRequestSync(const std::string &url,const std::string &method,const std::string &data);
bool Raw_HttpRequestAsync(const std::string &url,const std::string &method,const std::string &data,std::function<void(int,std::string)> callback);

unsigned int Raw_GetSystemThreadIdFromStdThread(std::thread::id id);
bool Raw_KillThread(DWORD id);