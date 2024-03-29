﻿#include "EventAPI.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <exception>
#include <cstdarg>
#include <Kernel/Base.h>
#include <Kernel/Block.h>
#include <Kernel/Player.h>
#include "EngineOwnData.h"
#include "EngineGlobalData.h"
#include "APIhelp.h"
#include "BlockAPI.h"
#include "ItemAPI.h"
#include "EntityAPI.h"
#include "PlayerAPI.h"
#include <Configs.h>
using namespace std;
using namespace script;

//////////////////// Helper Funcs ////////////////////

bool ProcessDebugEngine(const string& cmd);
bool CallPlayerCmdCallback(Player *player, const string& cmd);
bool CallServerCmdCallback(const string& cmd);
bool CallFormCallback(int formId, const string& data);

//////////////////// Listeners ////////////////////

enum class EVENT_TYPES : int
{
    OnJoin=0, OnLeft, OnPlayerCmd, OnChat, 
    OnRespawn, OnChangeDim, OnJump, OnSneak, OnAttack, OnEat, OnMove, OnSetArmor,
    OnUseItem, OnTakeItem, OnDropItem,
    OnDestroyingBlock, OnDestroyBlock, OnPlaceBlock,
    OnOpenChest, OnCloseChest, OnOpenBarrel, OnCloseBarrel, OnChangeSlot,
    OnMobDie, OnMobHurt, OnExplode, OnBlockExploded, OnCmdBlockExecute,
    OnProjectileHit, OnInteractdWith, OnUseRespawnAnchor, OnFarmLandDecay,
    OnPistonPush, OnHopperSearchItem, OnHopperPushOut,
    OnServerStarted, OnServerCmd, OnFormSelected, OnConsoleOutput,
    EVENT_COUNT
};
static const std::unordered_map<string, EVENT_TYPES> EventsMap{
    {"onJoin",EVENT_TYPES::OnJoin},
    {"onLeft",EVENT_TYPES::OnLeft},
    {"onPlayerCmd",EVENT_TYPES::OnPlayerCmd},
    {"onChat",EVENT_TYPES::OnChat},
    {"onRespawn",EVENT_TYPES::OnRespawn},
    {"onChangeDim",EVENT_TYPES::OnChangeDim},
    {"onJump",EVENT_TYPES::OnJump},
    {"onSneak",EVENT_TYPES::OnSneak},
    {"onAttack",EVENT_TYPES::OnAttack},
    {"onEat",EVENT_TYPES::OnEat},
    {"onMove",EVENT_TYPES::OnMove},
    {"onSetArmor",EVENT_TYPES::OnSetArmor},
    {"onMobDie",EVENT_TYPES::OnMobDie},
    {"onMobHurt",EVENT_TYPES::OnMobHurt},
    {"onUseItem",EVENT_TYPES::OnUseItem},
    {"onTakeItem",EVENT_TYPES::OnTakeItem},
    {"onDropItem",EVENT_TYPES::OnDropItem},
    {"onDestroyingBlock",EVENT_TYPES::OnDestroyingBlock},
    {"onDestroyBlock",EVENT_TYPES::OnDestroyBlock},
    {"onPlaceBlock",EVENT_TYPES::OnPlaceBlock},
    {"onExplode",EVENT_TYPES::OnExplode},
    {"onBlockExploded",EVENT_TYPES::OnBlockExploded},
    {"onOpenChest",EVENT_TYPES::OnOpenChest},
    {"onCloseChest",EVENT_TYPES::OnCloseChest},
    {"onOpenBarrel",EVENT_TYPES::OnOpenBarrel},
    {"onCloseBarrel",EVENT_TYPES::OnCloseBarrel},
    {"onChangeSlot",EVENT_TYPES::OnChangeSlot},
    {"onCmdBlockExecute",EVENT_TYPES::OnCmdBlockExecute},
    {"onProjectileHit",EVENT_TYPES::OnProjectileHit},
    {"onInteractdWith",EVENT_TYPES::OnInteractdWith},
    {"onUseRespawnAnchor",EVENT_TYPES::OnUseRespawnAnchor},
    {"onFarmLandDecay",EVENT_TYPES::OnFarmLandDecay},
    {"onPistonPush",EVENT_TYPES::OnPistonPush},
    {"onHopperSearchItem",EVENT_TYPES::OnHopperSearchItem},
    {"onHopperPushOut",EVENT_TYPES::OnHopperPushOut},
    {"onServerStarted",EVENT_TYPES::OnServerStarted},
    {"onServerCmd",EVENT_TYPES::OnServerCmd},
    {"onConsoleOutput",EVENT_TYPES::OnConsoleOutput},
    {"onFormSelected",EVENT_TYPES::OnFormSelected},
};
struct ListenerListType
{
    ScriptEngine *engine;
    Global<Function> func;
};
static std::vector<ListenerListType> listenerList[int(EVENT_TYPES::EVENT_COUNT)];

// Call Event
#define CallEvent(TYPE,...) \
    std::vector<ListenerListType> &nowList = listenerList[int(TYPE)]; \
    for(int i = 0; i < nowList.size(); ++i) { \
        EngineScope enter(nowList[i].engine); \
        try{ \
            nowList[i].func.get().call({},__VA_ARGS__); \
        } \
        catch(const Exception& e) \
        { \
            ERROR("Event Callback Failed!"); \
            ERRPRINT(e.message()); \
        } \
    }

#define CallEventEx(TYPE,...) \
    std::vector<ListenerListType> &nowList = listenerList[int(TYPE)]; \
    bool passToBDS = true; \
    for(int i = 0; i < nowList.size(); ++i) { \
        EngineScope enter(nowList[i].engine); \
        try{ \
            auto result = nowList[i].func.get().call({},__VA_ARGS__); \
            if(result.isBoolean() && result.asBoolean().value() == false) \
                passToBDS = false; \
        } \
        catch(const Exception& e) \
        { \
            ERROR("Event Callback Failed!"); \
            ERRPRINT(e.message()); \
        } \
    }\
    if(!passToBDS) { return false; }

#define CallEventRtn(TYPE,RETURN_VALUE,...) \
    std::vector<ListenerListType> &nowList = listenerList[int(TYPE)]; \
    bool passToBDS = true; \
    for(int i = 0; i < nowList.size(); ++i) { \
        EngineScope enter(nowList[i].engine); \
        try{ \
            auto result = nowList[i].func.get().call({},__VA_ARGS__); \
            if(result.isBoolean() && result.asBoolean().value() == false) \
                passToBDS = false; \
        } \
        catch(const Exception& e) \
        { \
            ERROR("Event Callback Failed!"); \
            ERRPRINT(e.message()); \
        } \
    }\
    if(!passToBDS) { return RETURN_VALUE; }


//////////////////// APIs ////////////////////

Local<Value> Listen(const Arguments& args)
{
    CHECK_ARGS_COUNT(args,2)
    CHECK_ARG_TYPE(args[0],ValueKind::kString)
    CHECK_ARG_TYPE(args[1],ValueKind::kFunction)

    try{
        int eventId = int(EventsMap.at(args[0].asString().toString()));
        Global<Function> func{args[1].asFunction()};
        listenerList[eventId].push_back({EngineScope::currentEngine(),func});
        return Boolean::newBoolean(true);
    }
    catch(const std::logic_error& e)
    {
        ERROR("Event \""+ args[0].asString().toString() +"\" No Found!\n");
        return Boolean::newBoolean(false);
    }
    CATCH("Fail to bind listener!")
}

//注册后台调试命令
void RegisterBuiltinCmds()
{
    static bool hasDone = false;
    if (!hasDone)
    {
        Raw_RegisterCmd(LXL_DEBUG_CMD, string(LXL_SCRIPT_LANG_TYPE) + " Engine Real-time Debugging", 4);
        DEBUG("Builtin Cmds Registered.");
        hasDone = true;
    }
}


//////////////////// Hook ////////////////////

#define IF_EXIST(EVENT) if(!listenerList[int(EVENT)].empty())

void InitEventListeners()
{
// ===== onPlayerJoin =====
    Event::addEventListener([](JoinEV ev)
    {
        IF_EXIST(EVENT_TYPES::OnJoin)
        {
            CallEvent(EVENT_TYPES::OnJoin, PlayerClass::newPlayer(ev.Player));
        }
    });

// ===== onPlayerLeft =====
    Event::addEventListener([](LeftEV ev)
    {
        IF_EXIST(EVENT_TYPES::OnLeft)
        {
            CallEvent(EVENT_TYPES::OnLeft, PlayerClass::newPlayer(ev.Player));
        }
    });

// ===== onChat =====
    Event::addEventListener([](ChatEV ev)
    {
        IF_EXIST(EVENT_TYPES::OnChat)
        {
            CallEventEx(EVENT_TYPES::OnChat, PlayerClass::newPlayer(ev.pl), ev.msg);
        }
        return true;
    });

// ===== onChangeDimension =====
    Event::addEventListener([](ChangeDimEV ev)
    {
        IF_EXIST(EVENT_TYPES::OnChangeDim)
        {
            CallEvent(EVENT_TYPES::OnChangeDim, PlayerClass::newPlayer(ev.Player));
        }
    });

// ===== onCmdBlockExecute =====
    Event::addEventListener([](CmdBlockExeEV ev)
    {
        IF_EXIST(EVENT_TYPES::OnCmdBlockExecute)
        {
            CallEventEx(EVENT_TYPES::OnCmdBlockExecute, ev.cmd, IntPos::newPos(ev.bpos.x, ev.bpos.y, ev.bpos.z));
        }
        return true;
    });


// For RegisterCmd...
    Event::addEventListener([](RegCmdEV ev) {
		CMDREG::SetCommandRegistry(ev.CMDRg);
    });


// ===== onServerStarted =====
    Event::addEventListener([](ServerStartedEV ev)
    {
        //注册预置命令
        RegisterBuiltinCmds();

        IF_EXIST(EVENT_TYPES::OnServerStarted)
        {
            CallEvent(EVENT_TYPES::OnServerStarted);
        }
    });
}

// ===== onAttack =====
THook(bool, "?attack@Player@@UEAA_NAEAVActor@@@Z",
    Player* pl,  Actor* ac)
{
    IF_EXIST(EVENT_TYPES::OnAttack)
    {
        CallEventEx(EVENT_TYPES::OnAttack, PlayerClass::newPlayer(pl), EntityClass::newEntity(ac));
    }
    return original(pl, ac);
}

// ===== onEat =====
THook(void, "?eat@Player@@QEAAXAEBVItemStack@@@Z",
    Player* _this, ItemStack* eaten)
{
    IF_EXIST(EVENT_TYPES::OnEat)
    {
        CallEvent(EVENT_TYPES::OnEat, PlayerClass::newPlayer(_this), ItemClass::newItem(eaten));
    }
    return original(_this, eaten);
}

// ===== onMove =====
THook(void, "??0MovePlayerPacket@@QEAA@AEAVPlayer@@AEBVVec3@@@Z",
    void *_this, Player * pl, Vec3 * to)
{
    IF_EXIST(EVENT_TYPES::OnMove)
    {
        CallEvent(EVENT_TYPES::OnMove, PlayerClass::newPlayer(pl), FloatPos::newPos(*to, Raw_GetPlayerDimId(pl)));
    }
    return original(_this, pl, to);
}

// ===== onSetArmor =====
THook(void, "?setArmor@Player@@UEAAXW4ArmorSlot@@AEBVItemStack@@@Z",
    Player* _this, unsigned slot, ItemStack* it)
{
    IF_EXIST(EVENT_TYPES::OnSetArmor)
    {
        CallEvent(EVENT_TYPES::OnSetArmor, PlayerClass::newPlayer(_this), Number::newNumber((int)slot), ItemClass::newItem(it));
    }
    return original(_this, slot, it);
}

// ===== onRespawn =====
THook(bool, "?respawn@Player@@UEAAXXZ",
    Player* pl)
{
    IF_EXIST(EVENT_TYPES::OnRespawn)
    {
        CallEventEx(EVENT_TYPES::OnRespawn, PlayerClass::newPlayer(pl));
    }
    return original(pl);
}

// ===== onJump =====
THook(void, "?jumpFromGround@Player@@UEAAXXZ",
    Player* pl)
{
    IF_EXIST(EVENT_TYPES::OnJump)
    {
        CallEvent(EVENT_TYPES::OnJump, PlayerClass::newPlayer(pl));
    }
    return original(pl);
}

// ===== onSneak =====
THook(void, "?sendActorSneakChanged@ActorEventCoordinator@@QEAAXAEAVActor@@_N@Z",
    void* _this, Actor* ac, bool isSneaking)
{
    IF_EXIST(EVENT_TYPES::OnSneak)
    {
        CallEvent(EVENT_TYPES::OnSneak, PlayerClass::newPlayer((Player*)ac), Boolean::newBoolean(isSneaking));
    }
    return original(_this, ac, isSneaking);
}

// ===== onDropItem =====
THook(bool, "?drop@Player@@UEAA_NAEBVItemStack@@_N@Z",
    Player* _this, ItemStack* a2, bool a3)
{
    IF_EXIST(EVENT_TYPES::OnDropItem)
    {
        CallEventEx(EVENT_TYPES::OnDropItem, PlayerClass::newPlayer(_this), ItemClass::newItem(a2));
    }
    return original(_this, a2, a3);
}

// ===== onTakeItem =====
THook(bool, "?take@Player@@QEAA_NAEAVActor@@HH@Z",
    Player* _this, Actor* actor, int a2, int a3)
{
    IF_EXIST(EVENT_TYPES::OnTakeItem)       //################### 有无办法改成获取item ###################
    {
        CallEventEx(EVENT_TYPES::OnTakeItem, PlayerClass::newPlayer(_this), EntityClass::newEntity(actor));
    }
    return original(_this, actor, a2, a3);
}

// ===== onPlayerCmd =====
THook(void, "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVCommandRequestPacket@@@Z",
    ServerNetworkHandler* _this, NetworkIdentifier* id, void* pkt)
{
    IF_EXIST(EVENT_TYPES::OnPlayerCmd)
    {
        auto player = SymCall("?_getServerPlayer@ServerNetworkHandler@@AEAAPEAVServerPlayer@@AEBVNetworkIdentifier@@E@Z",
            Player*, void*, void*, char)(_this, id, *(char*)((uintptr_t)pkt + 16));
        if (player)
        {   
            // Player Command
            auto cmd = std::string(*(std::string*)((uintptr_t)pkt + 48));
            if (cmd.front() == '/')
                cmd = cmd.substr(1);
            
            bool callbackRes = CallServerCmdCallback(cmd);
            
            CallEvent(EVENT_TYPES::OnPlayerCmd, PlayerClass::newPlayer(player), cmd);
            if (!callbackRes)
                return;
        }
    }
    return original(_this, id, pkt);
}

// ===== onUseItem =====
THook(bool, "?useItemOn@GameMode@@UEAA_NAEAVItemStack@@AEBVBlockPos@@EAEBVVec3@@PEBVBlock@@@Z",
    void* _this,  ItemStack* item, BlockPos* bp, unsigned __int8 a4, Vec3* a5,  Block* bl)
{
    IF_EXIST(EVENT_TYPES::OnUseItem)
    {
        auto sp = *reinterpret_cast<Player**>(reinterpret_cast<unsigned long long>(_this) + 8);

        CallEventEx(EVENT_TYPES::OnUseItem, PlayerClass::newPlayer(sp), ItemClass::newItem(item),
            BlockClass::newBlock(bl,bp), IntPos::newPos(bp->x, bp->y, bp->z, WPlayer(*sp).getDimID()));
    }
    return original(_this, item, bp, a4, a5, bl);
}

// ===== onDestroyingBlock =====
THook(float, "?getDestroySpeed@Player@@QEBAMAEBVBlock@@@Z",
    Player* _this, Block* bl)
{
    IF_EXIST(EVENT_TYPES::OnDestroyingBlock)
    {
        CallEvent(EVENT_TYPES::OnDestroyingBlock, PlayerClass::newPlayer(_this), BlockClass::newBlock(bl));
    }
    return original(_this, bl);
}

// ===== onDestroyBlock =====
THook(bool, "?checkBlockDestroyPermissions@BlockSource@@QEAA_NAEAVActor@@AEBVBlockPos@@AEBVItemStackBase@@_N@Z",
    BlockSource * _this, Actor* pl, BlockPos* pos, ItemStack* a3, bool a4)
{
    IF_EXIST(EVENT_TYPES::OnDestroyBlock)
    {
        auto block = SymCall("?getBlock@BlockSource@@QEBAAEBVBlock@@AEBVBlockPos@@@Z", Block*, BlockSource*, BlockPos*)(_this, pos);

        CallEventEx(EVENT_TYPES::OnDestroyBlock, PlayerClass::newPlayer((Player*)pl), BlockClass::newBlock(block,pos,_this),
            IntPos::newPos(pos->x, pos->y, pos->z,Raw_GetBlockDimension(_this)));
    }
    return original(_this, pl, pos,a3, a4);
}

// ===== onPlaceBlock =====
THook(bool, "?mayPlace@BlockSource@@QEAA_NAEBVBlock@@AEBVBlockPos@@EPEAVActor@@_N@Z",
    BlockSource* bs, Block* bl, BlockPos* bp, unsigned __int8 a4, Actor* pl, bool a6)
{
    IF_EXIST(EVENT_TYPES::OnPlaceBlock)
    {
        CallEventEx(EVENT_TYPES::OnPlaceBlock, PlayerClass::newPlayer((Player*)pl), BlockClass::newBlock(bl,bp,bs), IntPos::newPos(bp->x, bp->y, bp->z, Raw_GetBlockDimension(bs)));
    }
    return original(bs, bl, bp, a4, pl, a6);
}

// ===== onOpenChest =====
THook(bool, "?use@ChestBlock@@UEBA_NAEAVPlayer@@AEBVBlockPos@@E@Z",
    void* _this, Player* pl , BlockPos* bp)
{
    IF_EXIST(EVENT_TYPES::OnOpenChest)
    {
        CallEventEx(EVENT_TYPES::OnOpenChest, PlayerClass::newPlayer(pl), IntPos::newPos(bp->x, bp->y, bp->z, Raw_GetPlayerDimId(pl)));
    }
    return original(_this, pl, bp);
}

// ===== onCloseChest =====
THook(bool, "?stopOpen@ChestBlockActor@@UEAAXAEAVPlayer@@@Z",
    void* _this, Player* pl)
{
    IF_EXIST(EVENT_TYPES::OnCloseChest)
    {
        //auto bp = (BlockPos*)((intptr_t*)_this - 204);
        auto bp = SymCall("?getPosition@BlockActor@@QEBAAEBVBlockPos@@XZ", BlockPos*, BlockActor*)((BlockActor*)_this);
        //################### 坐标错误 ###################

        CallEventEx(EVENT_TYPES::OnCloseChest, PlayerClass::newPlayer(pl), IntPos::newPos(bp->x, bp->y, bp->z, Raw_GetPlayerDimId(pl)));
    }
    return original(_this, pl);
}

// ===== onOpenBarrel =====
THook(bool, "?use@BarrelBlock@@UEBA_NAEAVPlayer@@AEBVBlockPos@@E@Z",
    void* _this, Player* pl, BlockPos* bp)
{
    IF_EXIST(EVENT_TYPES::OnOpenBarrel)
    {
        CallEventEx(EVENT_TYPES::OnOpenBarrel, PlayerClass::newPlayer(pl), IntPos::newPos(bp->x, bp->y, bp->z, Raw_GetPlayerDimId(pl)));
    }
    return original(_this, pl, bp);
}

// ===== onCloseBarrel =====
THook(bool, "?stopOpen@BarrelBlockActor@@UEAAXAEAVPlayer@@@Z",
    void* _this, Player* pl)
{
    IF_EXIST(EVENT_TYPES::OnCloseBarrel)
    {
        //auto bp = (BlockPos*)((intptr_t*)_this - 204);
        auto bp = SymCall("?getPosition@BlockActor@@QEBAAEBVBlockPos@@XZ", BlockPos*, BlockActor*)((BlockActor*)_this);

        CallEventEx(EVENT_TYPES::OnCloseBarrel, PlayerClass::newPlayer(pl), IntPos::newPos(bp->x, bp->y, bp->z, Raw_GetPlayerDimId(pl)));
        //################### 坐标错误 ###################
    }
    return original(_this, pl);
}

// ===== onChangeSlot =====
class LevelContainerModel;
THook(void, "?containerContentChanged@LevelContainerModel@@UEAAXH@Z",
    LevelContainerModel* a1, int a2)
{
    IF_EXIST(EVENT_TYPES::OnChangeSlot)     //############## 崩服 ##############
    {
        Actor* v3 = dAccess<Actor*, 208>(a1);
        BlockSource* bs = dAccess<BlockSource*, 840>(v3);
        //offset LevelContainerModel::_getContainer(LevelContainerModel *this) Line->BlockSource::getBlockEntity
        BlockPos* bpos = dAccess<BlockPos*, 216>(a1);
        Block* pBlk = SymCall("?getBlock@BlockSource@@QEBAAEBVBlock@@AEBVBlockPos@@@Z", Block*, BlockSource*, BlockPos*)(bs, bpos);
        auto v5 = (*(__int64(__fastcall**)(LevelContainerModel*))(*(uintptr_t*)a1 + 160i64))(a1);
        if (v5)
        {
            ItemStack* item = (ItemStack*)(*(__int64(__fastcall**)(__int64, uintptr_t))(*(uintptr_t*)v5 + 40i64))(v5, a2);
            int count = offItemStack::getCount(item);
            int slotNumber = a2;
            Actor* pl = v3;

            CallEvent(EVENT_TYPES::OnChangeSlot, PlayerClass::newPlayer((Player*)pl), BlockClass::newBlock(pBlk,bpos,bs), slotNumber, IntPos::newPos(bpos->x, bpos->y, bpos->z, Raw_GetBlockDimension(bs)), count != 0, ItemClass::newItem(item));
        }
    }
	return original(a1, a2);
}

// ===== onMobDie =====
THook(bool, "?die@Mob@@UEAAXAEBVActorDamageSource@@@Z",
    Mob* self, void* ads)
{
    IF_EXIST(EVENT_TYPES::OnMobDie)
    {
        char v83;
        auto v6 = *(void**)(*(__int64(__fastcall**)(void*, char*))(*(intptr_t*)ads + 64i64))(ads, &v83);
        auto level = offPlayer::getLevel(self);
        auto ac = SymCall("?fetchEntity@Level@@UEBAPEAVActor@@UActorUniqueID@@_N@Z"
            , Actor*, Level*, void*, bool)(level, v6, 0);

        CallEventEx(EVENT_TYPES::OnMobDie, EntityClass::newEntity(self), EntityClass::newEntity(ac));
    }
    return original(self, ads);
}

// ===== onMobHurt =====
THook(bool, "?_hurt@Mob@@MEAA_NAEBVActorDamageSource@@H_N1@Z",
    Mob* self, void* ads, int a1, bool a2, bool a3)
{
    IF_EXIST(EVENT_TYPES::OnMobHurt)
    {
        char v83;
        auto v6 = *(void**)(*(__int64(__fastcall**)(void*, char*))(*(intptr_t*)ads + 64i64))(ads, &v83);
        auto level = offPlayer::getLevel(self);
        auto ac = SymCall("?fetchEntity@Level@@UEBAPEAVActor@@UActorUniqueID@@_N@Z"
            , Actor*, Level*, void*, bool)(level, v6, 0);
        
        CallEventEx(EVENT_TYPES::OnMobHurt, EntityClass::newEntity(self), EntityClass::newEntity(ac));
    }
    return original(self, ads, a1, a2, a3);
}

// ===== onExplode =====
THook(bool, "?explode@Level@@UEAAXAEAVBlockSource@@PEAVActor@@AEBVVec3@@M_N3M3@Z",
    Level* _this, BlockSource* bs, Actor* actor, Vec3 pos, float a5, bool a6, bool a7, float a8, bool a9)
{
    IF_EXIST(EVENT_TYPES::OnExplode)
    {
        CallEventEx(EVENT_TYPES::OnExplode, EntityClass::newEntity(actor), FloatPos::newPos(pos.x, pos.y, pos.z));
    }
    return original(_this, bs, actor, pos, a5, a6, a7, a8, a9);
}

// ===== onBlockExploded =====
THook(void, "?onExploded@Block@@QEBAXAEAVBlockSource@@AEBVBlockPos@@PEAVActor@@@Z",
    Block* _this, BlockSource *bs, BlockPos *bp, Actor * actor)
{
    IF_EXIST(EVENT_TYPES::OnBlockExploded)
    {
        CallEvent(EVENT_TYPES::OnBlockExploded,BlockClass::newBlock(_this,bp,bs),IntPos::newPos(*bp,Raw_GetBlockDimension(bs)),EntityClass::newEntity(actor));
    }
    return original(_this, bs, bp, actor);
}

// ===== OnProjectileHit =====
THook(void, "?onProjectileHit@Block@@QEBAXAEAVBlockSource@@AEBVBlockPos@@AEBVActor@@@Z",
    Block* _this, BlockSource* bs, BlockPos* bp, Actor* actor)
{
    IF_EXIST(EVENT_TYPES::OnProjectileHit)
    {
        CallEvent(EVENT_TYPES::OnProjectileHit,BlockClass::newBlock(_this,bp,bs),IntPos::newPos(*bp, Raw_GetBlockDimension(bs)),EntityClass::newEntity(actor));
    }
    return original(_this, bs, bp, actor);
}

// ===== OnInteractdWith =====
THook(unsigned short, "?onBlockInteractedWith@VanillaServerGameplayEventListener@@UEAA?AW4EventResult@@AEAVPlayer@@AEBVBlockPos@@@Z",
    void* _this, Player* pl, BlockPos* bp)
{
    IF_EXIST(EVENT_TYPES::OnProjectileHit)
    {
        CallEventRtn(EVENT_TYPES::OnInteractdWith, 0, PlayerClass::newPlayer(pl), IntPos::newPos(*bp, Raw_GetPlayerDimId(pl)));
    }
    return original(_this, pl, bp);
}

// ===== OnUseRespawnAnchor =====
THook(bool, "?trySetSpawn@RespawnAnchorBlock@@CA_NAEAVPlayer@@AEBVBlockPos@@AEAVBlockSource@@AEAVLevel@@@Z",
    Player* pl, BlockPos* a2, BlockSource* a3, Level* a4)
{
    IF_EXIST(EVENT_TYPES::OnUseRespawnAnchor)
    {
        CallEventEx(EVENT_TYPES::OnUseRespawnAnchor,PlayerClass::newPlayer(pl),IntPos::newPos(*a2, Raw_GetBlockDimension(a3)));
    }
    return original(pl, a2, a3, a4);
}

// ===== OnFarmLandDecay =====
THook(void, "?transformOnFall@FarmBlock@@UEBAXAEAVBlockSource@@AEBVBlockPos@@PEAVActor@@M@Z",
    void *_this, BlockSource * bs, BlockPos * bp, Actor * ac, float a5)
{
    IF_EXIST(EVENT_TYPES::OnFarmLandDecay)
    {
        CallEvent(EVENT_TYPES::OnFarmLandDecay,IntPos::newPos(*bp, Raw_GetBlockDimension(bs)),EntityClass::newEntity(ac));
    }
    return original(_this,bs,bp,ac,a5);
}

// ===== OnPistonPush =====
THook(bool, "?_attachedBlockWalker@PistonBlockActor@@AEAA_NAEAVBlockSource@@AEBVBlockPos@@EE@Z",
    BlockActor* _this, BlockSource* bs, BlockPos* bp, unsigned a3, unsigned a4)
{
    IF_EXIST(EVENT_TYPES::OnPistonPush)         //############## 崩服 ##############
    {
        int dim = Raw_GetBlockDimension(bs);
        BlockPos* pistonPos = dAccess<BlockPos*, 44>(_this);

        IntVec4 blockPos{ bp->x,bp->y,bp->z,dim };
        Block* pushedBlock = Raw_GetBlockByPos(&blockPos);

        CallEventEx(EVENT_TYPES::OnPistonPush, IntPos::newPos(*pistonPos, dim), BlockClass::newBlock(pushedBlock, bp, dim));
    }
    return original(_this, bs, bp, a3, a4);
}

// ===== OnHopperSearchItem =====
THook(bool, "?_tryPullInItemsFromAboveContainer@Hopper@@IEAA_NAEAVBlockSource@@AEAVContainer@@AEBVVec3@@@Z",
    void* _this, BlockSource* bs, void* container, Vec3* pos)
{
    IF_EXIST(EVENT_TYPES::OnHopperSearchItem)
    {
        CallEventEx(EVENT_TYPES::OnHopperSearchItem, FloatPos::newPos(*pos, Raw_GetBlockDimension(bs)));
    }
    return original(_this, bs, container, pos);
}

// ===== OnHopperPushOut =====
THook(bool, "?_pushOutItems@Hopper@@IEAA_NAEAVBlockSource@@AEAVContainer@@AEBVVec3@@H@Z",
    void* _this, BlockSource* bs, void* container, Vec3* pos, int a5)
{
    IF_EXIST(EVENT_TYPES::OnHopperPushOut)
    {
        CallEventEx(EVENT_TYPES::OnHopperPushOut, FloatPos::newPos(*pos, Raw_GetBlockDimension(bs)));
    }
    return original(_this, bs, container, pos, a5);
}

// ===== onServerCmd =====
THook(bool, "?executeCommand@MinecraftCommands@@QEBA?AUMCRESULT@@V?$shared_ptr@VCommandContext@@@std@@_N@Z",
    MinecraftCommands* _this, unsigned int* a2, std::shared_ptr<CommandContext> x, char a4)
{
    Player* player = MakeSP(x->getOrigin());
    if (!player)
    {
        // Server Command
        string cmd = x->getCmd();
        if (cmd.front() == '/')
            cmd = cmd.substr(1);

        if (!ProcessDebugEngine(cmd))
            return false;

        IF_EXIST(EVENT_TYPES::OnServerCmd)
        {
            bool callbackRes = CallServerCmdCallback(cmd);

            CallEventEx(EVENT_TYPES::OnServerCmd, cmd);
            if(!callbackRes)
                return false;
        }
    }
    return original(_this, a2, x, a4);
}

// ===== onFormSelected =====
THook(void, "?handle@?$PacketHandlerDispatcherInstance@VModalFormResponsePacket@@$0A@@@UEBAXAEBVNetworkIdentifier@@AEAVNetEventCallback@@AEAV?$shared_ptr@VPacket@@@std@@@Z",
	void* _this, unsigned long long id, ServerNetworkHandler* handler, void* packet)
{
    //IF_EXIST(EVENT_TYPES::OnFormSelected)

    Player* p = handler->_getServerPlayer(*(NetworkIdentifier*)(void*)id, *(unsigned char*)packet);
    if (p)
    {
        unsigned formId = dAccess<unsigned>(packet,48);
        string data = dAccess<string>(packet,56);

        if (data.back() == '\n')
            data.pop_back();
            
        CallFormCallback(formId, data);
        // No CallEvent here
    }

    original(_this, id, handler, packet);
}

// ===== onConsoleOutput =====
THook(ostream&, "??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z",
    ostream& _this, const char* str, unsigned size)
{
    IF_EXIST(EVENT_TYPES::OnConsoleOutput)
    {
        CallEventRtn(EVENT_TYPES::OnConsoleOutput, _this, String::newString(string(str)));
    }
    return original(_this, str, size);
}


//////////////////// Helper Funcs ////////////////////

vector<string> SplitCmdParas(const string& paras)
{
    vector<string> res;
    string now, strInQuote = "";
    istringstream strIn(paras);
    while (strIn >> now)
    {
        if (!strInQuote.empty())
        {
            strInQuote = strInQuote + " " + now;
            if (now.back() == '\"')
            {
                strInQuote.pop_back();
                res.push_back(strInQuote.erase(0, 1));
                strInQuote = "";
            }
        }
        else
        {
            if (now.front() == '\"')
                strInQuote = now;
            else
                res.push_back(now);
        }
    }
    if (!strInQuote.empty())
    {
        istringstream leftIn(strInQuote);
        while (leftIn >> now)
            res.push_back(now);
    }
    return res;
}

bool ProcessDebugEngine(const string& cmd)
{
    #define OUTPUT_DEBUG_SIGN() std::cout << "LiteXLoader-" << LXL_SCRIPT_LANG_TYPE << ">" << std::flush
    extern bool globalDebug;
    extern std::shared_ptr<ScriptEngine> debugEngine;

    if (cmd == LXL_DEBUG_CMD)
    {
        if (globalDebug)
        {   
            //EndDebug
            INFO("Debug mode ended");
            globalDebug = false;
        }
        else
        {   
            //StartDebug
            INFO("Debug mode begin");
            globalDebug = true;
            OUTPUT_DEBUG_SIGN();
        }
        return false;
    }
    if (globalDebug)
    {
        EngineScope enter(debugEngine.get());
        try
        {
            if (cmd == "stop")
            {
                WARN("请先退出Debug实时调试模式再使用stop！");
            }
            else
            {
                auto result = debugEngine->eval(cmd);
                PrintValue(std::cout, result);
                cout << endl;
                OUTPUT_DEBUG_SIGN();
            }
        }
        catch (Exception& e)
        {
            ERRPRINT(e);
            OUTPUT_DEBUG_SIGN();
        }
        return false;
    }
    return true;
}

bool CallPlayerCmdCallback(Player* player, const string& cmd)
{
    bool passToOriginalCmdEvent = true;

    for (auto engine : lxlModules)
    {
        EngineScope enter(engine.get());
        auto funcs = &(ENGINE_OWN_DATA()->playerCmdCallbacks);
        if (!funcs->empty())
            for (auto iter = funcs->begin(); iter != funcs->end(); ++iter)
                if (cmd.find_first_of(iter->first) == 0)
                {
                    //Matched
                    auto paras = SplitCmdParas(cmd.substr(iter->first.size() + 1) + " ");
                    Local<Array> args = Array::newArray({ String::newString(iter->first) });
                    for (string para : paras)
                        args.add(String::newString(para));

                    auto res = iter->second.get().call({}, PlayerClass::newPlayer(player), args);
                    if (res.isNull() || (res.isBoolean() && res.asBoolean().value() == false))
                        passToOriginalCmdEvent = false;
                    break;
                }
    }
    return passToOriginalCmdEvent;
}

bool CallServerCmdCallback(const string& cmd)
{
    bool passToOriginalCmdEvent = true;

    for (auto engine : lxlModules)
    {
        EngineScope enter(engine.get());
        auto funcs = &(ENGINE_OWN_DATA()->consoleCmdCallbacks);
        if (!funcs->empty())
            for (auto iter = funcs->begin(); iter != funcs->end(); ++iter)
                if (cmd.find_first_of(iter->first) == 0)
                {
                    //Matched
                    auto paras = SplitCmdParas(cmd.substr(iter->first.size() + 1));
                    Local<Array> args = Array::newArray({ String::newString(iter->first) });
                    for (string para : paras)
                        args.add(String::newString(para));

                    auto res = iter->second.get().call({}, args);
                    if (res.isNull() || (res.isBoolean() && res.asBoolean().value() == false))
                        passToOriginalCmdEvent = false;
                    break;
                }
    }
    return passToOriginalCmdEvent;
}

bool CallFormCallback(int formId, const string& data)
{
    bool passToBDS = true;
    auto callback = formCallbacks[formId];

    EngineScope scope(callback.first);
    auto res = callback.second.get().call({}, String::newString(data));
    if (res.isBoolean() && res.asBoolean().value() == false)
        passToBDS = false;

    return passToBDS;
}