#include "Db.h"
#include "Global.h"
#include <API/APIhelp.h>
#include <filesystem>
#include "ThirdParty.h"
using namespace std;

DB_ROOT Raw_NewDB(const string &dir)
{
    if(!filesystem::is_directory(dir))
        return nullptr;
    else
    {
        try
        {
            return MakeKVDB(dir);
        }
        catch(...)
        {
            return nullptr;
        }
    }
}

bool Raw_DBGet(DB_ROOT &db, const string & key, string &val)
{
    return db->get(key,val);
}

bool Raw_DBSet(DB_ROOT &db, const string & key, const string & val)
{
    db->put(key,val);
    return true;
}

bool Raw_DBDel(DB_ROOT &db, const string & key)
{
    db->del(key);
    return true;
}


vector<string> keyList;

vector<string> Raw_DBListKey(DB_ROOT db)
{
    keyList.clear();
    db->iter([](const string_view& key)
    {
        keyList.push_back(string(key));
        return true;
    });
    return keyList;
}

INI_ROOT Raw_IniOpen(const string &path)
{
    auto root = new SimpleIni;
    root->SetUnicode(true);
    auto res = root->LoadFile(path.c_str());
    if (res<0)
    {
        ERROR("Failed in loading ini file");
        ERRPRINT(res);
        delete root;
        return nullptr;
    }
    else
    {
        root->filePath = path;
        return root;
    }
}

bool Raw_IniClose(INI_ROOT ini)
{
    if(ini)
        delete ini;
    return true;
}

bool Raw_IniSetInt(INI_ROOT ini, const string & sec, const string & key, int value)
{
    if (!ini)
        return false;
    bool isOk = ini->SetLongValue(sec.c_str(), key.c_str(), value) >= 0;
    ini->SaveFile(ini->filePath.c_str());
    return isOk;
}

bool Raw_IniSetFloat(INI_ROOT ini, const string & sec, const string & key, float value)
{
    if (!ini)
        return false;
    bool isOk = ini->SetDoubleValue(sec.c_str(),key.c_str(),value) >= 0;
    ini->SaveFile(ini->filePath.c_str());
    return isOk;
}

bool Raw_IniSetString(INI_ROOT ini, const string & sec, const string & key, const string & value)
{
    if (!ini)
        return false;
    bool isOk = ini->SetValue(sec.c_str(),key.c_str(),value.c_str()) >= 0;
    ini->SaveFile(ini->filePath.c_str());
    return isOk;
}

bool Raw_IniSetBool(INI_ROOT ini, const string & sec, const string & key, bool value)
{
    if (!ini)
        return false;
    bool isOk = ini->SetBoolValue(sec.c_str(),key.c_str(),value) >= 0;
    ini->SaveFile(ini->filePath.c_str());
    return isOk;
}

int Raw_IniGetInt(INI_ROOT ini, const string & sec, const string & key, int def)
{
    if (!ini)
        return def;
    return ini->GetLongValue(sec.c_str(),key.c_str(),def);
}

float Raw_IniGetFloat(INI_ROOT ini, const string & sec, const string & key, float def)
{
    if (!ini)
        return def;
    return (float)ini->GetDoubleValue(sec.c_str(),key.c_str(),def);
}

string Raw_IniGetString(INI_ROOT ini, const string & sec, const string & key, const string & def)
{
    if (!ini)
        return def;
    return ini->GetValue(sec.c_str(),key.c_str(),def.c_str());
}

bool Raw_IniGetBool(INI_ROOT ini, const string & sec, const string & key, bool def)
{
    if (!ini)
        return def;
    return ini->GetBoolValue(sec.c_str(),key.c_str(),def);
}

bool Raw_IniDeleteKey(INI_ROOT ini, const std::string & sec, const std::string & key)
{
    if (!ini)
        return false;
    return ini->Delete(sec.c_str(),key.c_str(),true);
}


////////////// Helper //////////////
typedef money_t (*Dy_GetMoney_T)(xuid_t);
typedef string (*Dy_GetTransHist_T)(xuid_t, int);
typedef bool (*Dy_TransMoney_T)(xuid_t, xuid_t, money_t, string const&);
typedef bool (*Dy_SetMoney_T)(xuid_t, money_t);
typedef bool (*Dy_AddMoney_T)(xuid_t, money_t);
typedef bool (*Dy_ReduceMoney_T)(xuid_t, money_t);
typedef void (*Dy_ClearHist_T)(int);

struct dynamicSymbolsMap_type
{
    Dy_GetMoney_T Dy_GetMoney = nullptr;
    Dy_SetMoney_T Dy_SetMoney = nullptr;
    Dy_AddMoney_T Dy_AddMoney = nullptr;
    Dy_ReduceMoney_T Dy_ReduceMoney = nullptr;
    Dy_TransMoney_T Dy_TransMoney = nullptr;
    Dy_GetTransHist_T Dy_GetTransHist = nullptr;
    Dy_ClearHist_T Dy_ClearHist = nullptr;
} dynamicSymbolsMap;
////////////// Helper //////////////

bool Raw_InitEcnonmicSystem()
{
    auto libs = liteloader::getAllLibs();
    bool libExists = false;
    for (auto& [name, h] : libs)
        if (name.find(L"LLMoney.dll") != wstring::npos)
        {
            libExists = true;
                
            dynamicSymbolsMap.Dy_GetMoney = (Dy_GetMoney_T)GetProcAddress(h, "llgetMoney");
            if(!dynamicSymbolsMap.Dy_GetMoney)
                WARN("Fail to load API money.getMoney!");

            dynamicSymbolsMap.Dy_SetMoney = (Dy_SetMoney_T)GetProcAddress(h, "llsetMoney");
            if (!dynamicSymbolsMap.Dy_SetMoney)
                WARN("Fail to load API money.setMoney!");

            dynamicSymbolsMap.Dy_AddMoney = (Dy_AddMoney_T)GetProcAddress(h, "lladdMoney");
            if (!dynamicSymbolsMap.Dy_AddMoney)
                WARN("Fail to load API money.addMoney!");

            dynamicSymbolsMap.Dy_ReduceMoney = (Dy_ReduceMoney_T)GetProcAddress(h, "llreduceMoney");
            if (!dynamicSymbolsMap.Dy_ReduceMoney)
                WARN("Fail to load API money.reduceMoney!");

            dynamicSymbolsMap.Dy_TransMoney = (Dy_TransMoney_T)GetProcAddress(h, "llcreateTrans");
            if (!dynamicSymbolsMap.Dy_TransMoney)
                WARN("Fail to load API money.transMoney!");

            dynamicSymbolsMap.Dy_GetTransHist = (Dy_GetTransHist_T)GetProcAddress(h, "llgetTransHist");
            if (!dynamicSymbolsMap.Dy_GetTransHist)
                WARN("Fail to load API money.getTransHist!");

            dynamicSymbolsMap.Dy_ClearHist = (Dy_ClearHist_T)GetProcAddress(h, "llpurgeHist");
            if (!dynamicSymbolsMap.Dy_ClearHist)
                WARN("Fail to load API money.clearHist!");
        }
    if (!libExists)
        WARN("LLMoney no found! Economic system in LXL will not work.");
}

money_t Raw_GetMoney(xuid_t player)
{
    if (dynamicSymbolsMap.Dy_GetMoney)
        return dynamicSymbolsMap.Dy_GetMoney(player);
    else
    {
        ERROR("API money.getMoney have not been loaded!");
        return -1;
    }
}

bool Raw_SetMoney(xuid_t player, money_t money)
{
    if (dynamicSymbolsMap.Dy_SetMoney)
        return dynamicSymbolsMap.Dy_SetMoney(player,money);
    else
    {
        ERROR("API money.setMoney have not been loaded!");
        return -1;
    }
}

bool Raw_AddMoney(xuid_t player, money_t money)
{
    if (dynamicSymbolsMap.Dy_AddMoney)
        return dynamicSymbolsMap.Dy_AddMoney(player,money);
    else
    {
        ERROR("API money.addMoney have not been loaded!");
        return -1;
    }
}

bool Raw_ReduceMoney(xuid_t player, money_t money)
{
    if (dynamicSymbolsMap.Dy_ReduceMoney)
        return dynamicSymbolsMap.Dy_ReduceMoney(player, money);
    else
    {
        ERROR("API money.reduceMoney have not been loaded!");
        return -1;
    }
}

bool Raw_TransMoney(xuid_t player1, xuid_t player2, money_t money, string const& notes)
{
    if (dynamicSymbolsMap.Dy_TransMoney)
        return dynamicSymbolsMap.Dy_TransMoney(player1, player2, money, notes);
    else
    {
        ERROR("API money.transMoney have not been loaded!");
        return -1;
    }
}

std::string Raw_GetMoneyHist(xuid_t player, int time)
{
    if (dynamicSymbolsMap.Dy_GetTransHist)
        return dynamicSymbolsMap.Dy_GetTransHist(player, time);
    else
    {
        ERROR("API money.getTransHist have not been loaded!");
        return "";
    }
}

bool Raw_ClearMoneyHist(int time)
{
    if (dynamicSymbolsMap.Dy_ClearHist)
    {
        dynamicSymbolsMap.Dy_ClearHist(time);
        return true;
    }
    else
    {
        ERROR("API money.clearHist have not been loaded!");
        return false;
    }
}
