# LiteXLoader - 玩家 API
> 对于实体对象，除了可以使用 **通用API** 和 **生物API** 之外，还有这些额外的功能  

### 获取指定玩家的玩家指针  
`getPlayer(info)`
- 参数：
    - info : `String`  
    玩家的名字或者Xuid  
- 返回值：玩家指针  
- 返回值类型：`Pointer` 
    - 如返回值为 `Null` 则表示获取玩家失败  
<br>

### 获取玩家名字  
`getName(player)`
- 参数：
    - player : `Pointer`  
    待查询的玩家指针  
- 返回值：目标玩家的名字
- 返回值类型： `String` 
    - 如返回值为 `Null` 则表示获取名字失败  
<br>

### 获取玩家Xuid  
`getXuid(player)`
- 参数：
    - player : `Pointer`  
    玩家指针  
- 返回值：玩家的Xuid  
- 返回值类型：`String` 
    - 如返回值为 `Null` 则表示获取Xuid失败  
<br>

### 获取玩家坐标  
`getPos(player)`
- 参数：
    - player : `Pointer`  
    待查询的玩家指针  
- 返回值：目标玩家的位置
- 返回值类型：`FloatPos` 
    - 如返回值为 `Null` 则表示获取位置失败  
<br>

### 获取玩家真实名字（无法被篡改）  
`getRealName(player)`
- 参数：
    - player : `Pointer`  
    待查询的玩家指针  
- 返回值：目标玩家的IP地址
- 返回值类型： `String` 
    - 如返回值为 `Null` 则表示获取IP失败  
<br>

### 获取玩家IP地址  
`getIP(player)`
- 参数：
    - player : `Pointer`  
    待查询的玩家指针  
- 返回值：目标玩家的真实名字
- 返回值类型： `String` 
    - 如返回值为 `Null` 则表示获取名字失败  
<br>

### 获取在线玩家列表  
`getPlayerList()`
- 参数：
    - 无  
- 返回值：在线的玩家列表（一个由玩家指针组成的数组）
- 返回值类型：`Array<Pointer,Pointer,...>`  
<br>

### 判断玩家是否为OP  
`isOP(player)`
- 参数：
    - player : `Pointer`  
    待查询的玩家指针  
- 返回值：是否为OP
- 返回值类型：`Boolean`  
    - 如返回值为 `Null` 则表示获取玩家信息失败  
<br>

### 查询玩家操作权限等级  
`getPlayerPermLevel(player)`
- 参数：
    - player : `Pointer`  
    待查询的玩家指针  
- 返回值：目标玩家的操作权限
    - 0 : Normal
    - 1 : Privileged
    - 2 : AutomationPlayer
    - 3 : OperatorOnly
    - 4 : ConsoleOnly  
- 返回值类型：`Number`
<br>

### 修改玩家操作权限等级  
`setPlayerPermLevel(player,level)`
- 参数：
    - player : `Pointer`  
    待修改的玩家指针  
    - level : `Number`
    目标操作权限等级
        - 0 : Normal
        - 1 : Privileged
        - 2 : AutomationPlayer
        - 3 : OperatorOnly
        - 4 : ConsoleOnly  
- 返回值：是否成功修改
- 返回值类型：`Boolean`  
<br>

### 断开指定玩家连接  
`kickPlayer(player[,msg])`
- 参数：
    - player : `Pointer`  
    玩家指针  
    - msg(可选参数) : `String`
    被踢出玩家出显示的断开原因。默认为“正在断开连接”  
- 返回值：是否成功断开连接
- 返回值类型：`Boolean`  
<br>

### 发送一个文本给玩家  
`tell(player,msg[,type])`
- 参数：
    - player : `Pointer`  
    玩家指针
    - msg : `String`  
    待发送的文本  
    - type(可选参数) : `Number`
    发送的文本消息类型，默认为0
        - 0 Raw
        - 1 Chat
        - 2 Translation
        - 3 Popup
        - 4 JukeboxPopup
        - 5 Tip
        - 6 System
        - 7 Whisper
        - 8 Announcement
        - 9 Json
- 返回值：是否成功发送
- 返回值类型：`Boolean`  
<br>

### 传送玩家至指定位置  
`teleport(player,pos)`
- 参数：
    - player : `Pointer`  
    玩家指针
    - pos : `FloatPos`  
    目标位置坐标 
- 返回值：是否成功传送
- 返回值类型：`Boolean`  
<br>

### 传送玩家至指定服务器  
`transServer(player,server,port)`
- 参数：
    - player : `Pointer`  
    玩家指针
    - server : `String`
    目标服务器IP / 域名
    - port : `Number`
    目标服务器端口  
- 返回值：是否成功传送
- 返回值类型：`Boolean` 
<br>

### 查询玩家背包  
`getPack(player,)`
- 参数：
    - player : `Pointer`  
    玩家指针
- 返回值：
- 返回值类型：  
<br>

### 获取玩家手持物品  
`getHand(player)`
- 参数：
    - player : `Pointer`  
    玩家指针
- 返回值：手持的物品指针
- 返回值类型：`Pointer`  
<br>

### 以指定玩家身份执行一条指令  
`runCmdAs(player,cmd)`
- 参数：
    - player : `Pointer`  
    玩家指针  
    - cmd : `String`  
    待执行的命令  
- 返回值：是否执行成功
- 返回值类型： `Boolean`   
<br>

### 杀死玩家  
`kill(player,pos)`
- 参数：
    - player : `Pointer`  
    玩家指针  
- 返回值：是否成功执行
- 返回值类型：`Boolean`   
<br>

### 重命名玩家  
`renamePlayer(player,newname)`
- 参数：
    - player : `Pointer`  
    玩家指针
    - newname : `String`  
    玩家的新名字  
- 返回值：是否重命名成功
- 返回值类型：`Boolean`  
<br>