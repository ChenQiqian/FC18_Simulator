#include "definition.h"

#ifndef _GAME_H_
#    define _GAME_H_
class Game : public Info {
  private:
    int                      id;        // 地图编号
    TMap                     m_width;   //【FC18】地图宽度
    TMap                     m_height;  //【FC18】地图高度
    vector<vector<mapBlock>> gameMap;
    vector<vector<mapBlock>> _gameMap_backup;  // 考虑到可能有塔变成平地的情况

  public:
    Game(TMap _width, TMap _height, TPlayer _players, int _id);
    Game(Json::Value json, bool game);
    ~Game();
    //【FC18】获取地图宽度
    TMap getWidth() const;
    //【FC18】获取地图高度
    TMap getHeight() const;
    void generateMap();   // 生成地图的地形到 gameMap
    void generateUser();  // 生成 playerInfo 数组
    void generateTower();  // 生成地图上最初的四个塔. 调用 addTower函数。
    mapBlock &block(TPoint p);  // 获取地图上的一个 block
    // 辅助判定函数
    bool isPosValid(TPoint p);
    bool isPosValid(int x, int y);
    bool isMyTower(TPlayerID pid, TTowerID tid);
    bool isMyCorps(TPlayerID pid, TCorpsID cid);
    bool isMyCell(TPlayerID pid, TPoint p);
    // 辅助计算函数
    int         needMoveCost(TPoint a, TPoint b);
    int         countCorpsNum(TPlayerID pid, corpsType type);
    TDoublePara getCorpsForce(const CorpsInfo &corps);
    TDoublePara getTowerForce(const TowerInfo &tower);
    // 辅助操作函数
    void deadCorps(TCorpsID cid);
    void getCorps(TCorpsID cid, TPlayerID pid);
    void addTower(TPlayerID pid, TPoint p);
    void addBattle(TPlayerID pid, TPoint p, battleCorpsType type);
    void addConstruct(TPlayerID pid, TPoint p, constructCorpsType type);
    void changeTowerLevel(TTowerID tid, TLevel tarLevel, bool fullBlood = 0);
    void deadTower(TTowerID tid);
    void getTower(TTowerID tid, TPlayerID pid);
    // 塔相关的命令
    void addtowerProduct(TowerInfo &tower, const vector<int> &par);
    void towerProduct(TowerInfo &tower);
    void towerAttackCorps(TowerInfo &tower, const vector<int> &par);
    // 兵团相关命令
    void corpsMove(CorpsInfo &corps, const vector<int> &par);
    void corpsAttackCorps(CorpsInfo &corps, const vector<int> &par);
    void corpsAttackTower(CorpsInfo &corps, const vector<int> &par);
    void corpsBuild(CorpsInfo &corps, const vector<int> &par);
    void corpsRepair(CorpsInfo &corps, const vector<int> &par);
    void corpsChangeTerrain(CorpsInfo &corps, const vector<int> &par);
    // 单条命令执行
    void doCommand(TPlayerID playerID, Command &todoCmd);
    // 为第 id 用户执行 todoCommandList, 更新到 Info 当中
    void operateCommandList(TPlayerID playerID, CommandList &todoCommandList);
    // 维护相关函数
    void updateProduct(TPlayerID pid);  // 维护生产力
    void updateTerrain();                // 维护地形。
    void updateMapOwner();  // 更新格子的归属。 全图更新，先都抹掉，再重新计算
    void updatePlayer();  // 更新玩家占领地盘和分数，判断玩家是否出局
    void updateInfo();  // 回合之后统一的update
    void updateFromInfo(const Info &info);
    // 刷新兵团建设力和行动力
    void        refresh();
    Json::Value play(TPlayerID playerID, CommandList &todoCommandList);
    void        print();
    Json::Value asJson();
};

#endif