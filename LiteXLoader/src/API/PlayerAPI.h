#pragma once
#include <ScriptX/ScriptX.h>
using namespace script;

//////////////////// Classes ////////////////////
class Player;
class PlayerClass : public ScriptClass
{
private:
    Player *player;
public:
	explicit PlayerClass(Player *p)
        :ScriptClass(ScriptClass::ConstructFromCpp<PlayerClass>{}),player(p)
    {}
    Player *get()
    {
        return player;
    }

    static Local<Object> newPlayer(Player *p);
    static Local<Object> newPlayer(WPlayer p);
    static Player* extractPlayer(Local<Value> v);

    Local<Value> getName();
    Local<Value> getPos();
    Local<Value> getRealName();
    Local<Value> getXuid();
    Local<Value> getUuid();
    Local<Value> getIP();
    Local<Value> getMaxHealth();
    Local<Value> getHealth();
    Local<Value> getInAir();
    Local<Value> getSneaking();

    Local<Value> isOP(const Arguments& args);
    Local<Value> getPermLevel(const Arguments& args);
    Local<Value> setPermLevel(const Arguments& args);

    Local<Value> runcmd(const Arguments& args);
    Local<Value> teleport(const Arguments& args);
    Local<Value> kill(const Arguments& args);
    Local<Value> kick(const Arguments& args);
    Local<Value> tell(const Arguments& args);
    Local<Value> getHand(const Arguments& args);
    Local<Value> getOffHand(const Arguments& args);
    Local<Value> getPack(const Arguments& args);
    Local<Value> rename(const Arguments& args);
    Local<Value> addLevel(const Arguments& args);
    Local<Value> transServer(const Arguments& args);

    Local<Value> getScore(const Arguments& args);
    Local<Value> setScore(const Arguments& args);
    Local<Value> addScore(const Arguments& args);
    Local<Value> removeScore(const Arguments& args);
    Local<Value> setScoreBoard(const Arguments& args);
    Local<Value> removeScoreBoard(const Arguments& args);
    Local<Value> setBossBar(const Arguments& args);
    Local<Value> removeBossBar(const Arguments& args);

    Local<Value> sendSimpleForm(const Arguments& args);
    Local<Value> sendModalForm(const Arguments& args);
    Local<Value> sendForm(const Arguments& args);

    Local<Value> setExtraData(const Arguments& args);
    Local<Value> getExtraData(const Arguments& args);
    Local<Value> delExtraData(const Arguments& args);
};

//////////////////// APIs ////////////////////

Local<Value> Broadcast(const Arguments& args);

Local<Value> GetPlayer(const Arguments& args);
Local<Value> GetOnlinePlayers(const Arguments& args);