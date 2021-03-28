#include "game.h"

Game::Game(TMap _width, TMap _height, TPlayer _players, int _id)
    : Info(_players), id(_id), m_width(_width), m_height(_height) {
    srand(id);
    gameMapInfo = &gameMap;
    generateMap();
    _gameMap_backup = gameMap;
    generateUser();
    generateTower();
    updateTerrain();
    updatePlayer();
}
Game::~Game() {}
//【FC18】获取地图宽度
TMap Game::getWidth() const {
    return m_width;
}
//【FC18】获取地图高度
TMap Game::getHeight() const {
    return m_height;
}
// 生成地图的地形到 gameMap
void Game::generateMap() {
    for(int i = 0; i < m_width; i++) {
        vector<mapBlock> _tmp_column;
        for(int j = 0; j < m_height; j++) {
            int t = rand() % 5 + 1;
            switch(t) {
                case 1:
                    _tmp_column.push_back(
                        mapBlock(TRPlain, PUBLIC, NOTOWER));
                    break;
                case 2:
                    _tmp_column.push_back(
                        mapBlock(TRMountain, PUBLIC, NOTOWER));
                    break;
                case 3:
                    _tmp_column.push_back(
                        mapBlock(TRForest, PUBLIC, NOTOWER));
                    break;
                case 4:
                    _tmp_column.push_back(
                        mapBlock(TRSwamp, PUBLIC, NOTOWER));
                    break;
                case 5:
                    _tmp_column.push_back(
                        mapBlock(TRRoad, PUBLIC, NOTOWER));
                    break;
                default: assert(0); break;
            }
        }
        gameMap.push_back(_tmp_column);
    }
}
// 生成 playerInfo 数组
void Game::generateUser() {
    for(TPlayerID pid = 1; pid <= totalPlayers; pid++) {
        playerInfo.push_back({pid});
    }
}
// 生成地图上最初的四个塔. 调用 addTower函数。
void Game::generateTower() {
    // 尽量在四周建造最初的塔!
    // 默认就四个角，5*5 的空当
    // 假装我已经知道只有四个玩家。
    addTower(1, TPoint({rand() % 5, rand() % 5}));
    addTower(2, TPoint({rand() % 5, 10 + rand() % 5}));
    addTower(3, TPoint({10 + rand() % 5, rand() % 5}));
    addTower(4, TPoint({10 + rand() % 5, 10 + rand() % 5}));
}
// 获取地图上的一个 block
mapBlock & Game::block(TPoint p) {
    return gameMap[p.m_y][p.m_x];
}
// 辅助判定函数
bool Game::isPosValid(TPoint p) {
    return isPosValid(p.m_x, p.m_y);
}  //判断点是否越界
bool Game::isPosValid(int x, int y) {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}
bool Game::isMyTower(TPlayerID pid, TTowerID tid) {
    return pid == towerInfo[tid].ownerID;
}
bool Game::isMyCorps(TPlayerID pid, TCorpsID cid) {
    return pid == corpsInfo[cid].owner;
}
bool Game::isMyCell(TPlayerID pid, TPoint p) {
    return pid == block(p).owner;
}
// 辅助计算函数
int Game::needMoveCost(TPoint a, TPoint b) {
    return ceil(double(CorpsMoveCost[block(a).type] +
                        CorpsMoveCost[block(b).type]) /
                    2 -
                1e-6);
}
int Game::countCorpsNum(TPlayerID pid, corpsType type) {
    int ans = 0;
    for(TCorpsID cid : playerInfo[pid - 1].corps) {
        if(corpsInfo[cid].type == type)
            ans++;
    }
    return ans;
}
TDoublePara Game::getCorpsForce(const CorpsInfo &corps) {
    return double(1) * double(corpsBattlePoint[corps.m_BattleType][0]) *
        double(corps.healthPoint) /
        double(battleHealthPoint[corps.m_BattleType][0]) +
        double(corpsTerrainGain[block(corps.pos).type]);
}
TDoublePara Game::getTowerForce(const TowerInfo &tower) {
    TDoublePara sumf = 0;
    for(TCorpsID cid : block(tower.pos).corps) {
        if(corpsInfo[cid].type == Battle) {
            sumf += corpsTowerGain[corpsInfo[cid].m_BattleType][0];
        }
    }
    TowerConfig conf = TowerInitConfig[tower.level - 1];
    return double(1) * conf.initBattlePoint * double(tower.healthPoint) /
        conf.initHealthPoint +
        sumf;
}
// 辅助操作函数
void Game::deadCorps(TCorpsID cid) {
    totalCorps--;
    CorpsInfo &corps = corpsInfo[cid];
    corps.exist      = false;
    // 地图
    vector<int> &mapCorps = block(corps.pos).corps;
    for(size_t i = 0; i < mapCorps.size(); i++) {
        if(mapCorps[i] == cid) {
            mapCorps.erase(mapCorps.begin() + i);
            break;
        }
    }
    // 玩家
    playerInfo[corps.owner - 1].corps.erase(cid);
}
void Game::getCorps(TCorpsID cid, TPlayerID pid) {
    // 给旧玩家删除
    deadCorps(cid);
    CorpsInfo &corps = corpsInfo[cid];
    if(countCorpsNum(pid, Construct) >= MAX_CONSTRUCT_NUM) {
        return;
    }
    totalCorps++;
    corps.exist = true;
    corps.owner = pid;
    block(corps.pos).corps.push_back(cid);
    playerInfo[pid - 1].corps.insert(cid);
}
void Game::addTower(TPlayerID pid, TPoint p) {
    if(playerInfo[pid - 1].tower.size() >= MAX_TOWER_NUM) {
        return;
    }
    totalTowers++;
    TTowerID tid = towerInfo.size();
    towerInfo.push_back({tid, pid, p});
    block(p).TowerIndex = tid;
    block(p).type       = TRTower;
    playerInfo[pid - 1].tower.insert(tid);
    updateMapOwner();
}
void Game::addBattle(TPlayerID pid, TPoint p, battleCorpsType type) {
    if(countCorpsNum(pid, Battle) >= MAX_BATTLE_NUM) {
        return;
    }
    TCorpsID cid = corpsInfo.size();
    corpsInfo.push_back({p, cid, pid, Battle, type, Explorer});
    totalCorps++;
    // update player
    block(p).corps.push_back(cid);
    playerInfo[pid - 1].corps.insert(cid);
}
void Game::addConstruct(TPlayerID pid, TPoint p, constructCorpsType type) {
    // cerr << "successfully add construct for " << pid << " at " << p.m_x
    // << "," << p.m_y << endl;
    if(countCorpsNum(pid, Construct) >= MAX_CONSTRUCT_NUM) {
        return;
    }
    TCorpsID cid = corpsInfo.size();
    corpsInfo.push_back({p, cid, pid, Construct, Archer, type});
    totalCorps++;
    // update player
    block(p).corps.push_back(cid);
    playerInfo[pid - 1].corps.insert(cid);
}
void Game::changeTowerLevel(TTowerID tid, TLevel tarLevel, bool fullBlood) {
    TowerInfo &tower = towerInfo[tid];
    // use a list initialization
    tower.productPoint = TowerInitConfig[tarLevel - 1].initProductPoint;
    tower.battlePoint  = TowerInitConfig[tarLevel - 1].initBattlePoint;
    if(!fullBlood)
        tower.healthPoint =
            floor(double(tower.healthPoint) /
                        TowerInitConfig[tower.level - 1].initHealthPoint *
                        TowerInitConfig[tarLevel - 1].initHealthPoint +
                    1e-6);
    else
        tower.healthPoint = TowerInitConfig[tarLevel - 1].initHealthPoint;
    tower.level = tarLevel;
}
void Game::deadTower(TTowerID tid) {
    TowerInfo &tower = towerInfo[tid];
    tower.exist      = false;
    totalTowers--;
    vector<int> _corp = block(tower.pos).corps;  // 不能传引用
    for(TCorpsID cid : _corp) {
        deadCorps(cid);
    }
    TPlayerID pid = tower.ownerID;
    playerInfo[pid - 1].tower.erase(tid);
    updateTerrain();
    updateMapOwner();
}
void Game::getTower(TTowerID tid, TPlayerID pid) {
    deadTower(tid);
    TowerInfo &tower = towerInfo[tid];

    if(tower.level - 4 <= 0) {
        return;
    }
    totalTowers++;
    tower.exist   = true;
    tower.ownerID = pid;
    playerInfo[pid - 1].tower.insert(tid);
    block(tower.pos).type = TRTower;
    changeTowerLevel(tid, tower.level - 4, 1);
    updateMapOwner();
}
// 塔相关的命令
void Game::addtowerProduct(TowerInfo &tower, const vector<int> &par) {
    tower.haveDoneCommand = true;
    switch(par[2]) {
        case PWarrior: tower.pdtType = PWarrior; break;
        case PArcher: tower.pdtType = PArcher; break;
        case PCavalry: tower.pdtType = PCavalry; break;
        case PBuilder: tower.pdtType = PBuilder; break;
        case PExplorer: tower.pdtType = PExplorer; break;
        case PUpgrade: tower.pdtType = PUpgrade; break;
        default: assert(0); break;
    }
    if(tower.pointsNeeded[tower.pdtType] == 0) {
        tower.pointsNeeded[tower.pdtType] = TowerProductCost[tower.pdtType];
        if(tower.pdtType == PUpgrade) {
            tower.pointsNeeded[tower.pdtType] *= tower.level;
        }
    }
    tower.productConsume = tower.pointsNeeded[tower.pdtType];
}
void Game::towerProduct(TowerInfo &tower) {
    // 生产上一回合剩下的
    // cerr << "test towerproduct";
    if(tower.pdtType == NOTASK)
        return;
    tower.pointsNeeded[tower.pdtType] -= tower.productPoint;
    if(tower.pointsNeeded[tower.pdtType] > 0) {
        return;
    }
    tower.pointsNeeded[tower.pdtType] = 0;
    switch(tower.pdtType) {
        case PWarrior:
        case PArcher:
        case PCavalry:
            if(countCorpsNum(tower.ownerID, Battle) >= MAX_BATTLE_NUM) {
                return;
            }
            if(tower.pdtType == PWarrior) {
                addBattle(tower.ownerID, tower.pos, Warrior);
            }
            if(tower.pdtType == PArcher) {
                addBattle(tower.ownerID, tower.pos, Archer);
            }
            if(tower.pdtType == PCavalry) {
                addBattle(tower.ownerID, tower.pos, Cavalry);
            }
            break;
        case PBuilder:
        case PExplorer:
            if(countCorpsNum(tower.ownerID, Construct) >=
                MAX_CONSTRUCT_NUM) {
                return;
            }
            if(tower.pdtType == PExplorer) {
                addConstruct(tower.ownerID, tower.pos, Explorer);
            }
            if(tower.pdtType == PBuilder) {
                addConstruct(tower.ownerID, tower.pos, Builder);
            }
            break;
        case PUpgrade: changeTowerLevel(tower.ID, tower.level + 1); break;
        default: assert(0); break;
    }
    tower.pdtType        = NOTASK;
    tower.productConsume = 0;
}
void Game::towerAttackCorps(TowerInfo &tower, const vector<int> &par) {
    TCorpsID tarID = par[2];
    if(tarID >= int(corpsInfo.size())) {
        return;
    }
    CorpsInfo &tar = corpsInfo[tarID];
    if(getDist(tower.pos, tar.pos) > 2) {
        return;
    }
    if(isMyCorps(tower.ownerID, tarID)) {
        return;
    }
    if(block(tar.pos).TowerIndex != -1) {
        return;
    }
    if(tar.type == Construct) {
        if(block(tar.pos).corps.size() > 1) {
            int real_tar = int(block(tar.pos).corps[0] != tarID);
            real_tar     = block(tar.pos).corps[real_tar];
            towerAttackCorps(tower, {TAttackCorps, tower.ID, real_tar});
        }
        else {
            tower.haveDoneCommand = true;
            getCorps(tarID, tower.ownerID);
        }
    }
    else if(tar.type == Battle) {
        tower.haveDoneCommand = true;
        TDoublePara fa = getTowerForce(tower), fb = getCorpsForce(tar);
        TDoublePara myHealthLost  = 0,
                    tarHealthLost = (30 * exp(0.04 * (fa - fb)));
        tar.healthPoint -= tarHealthLost;
        if(tar.healthPoint < 0) {
            deadCorps(tarID);
            if(block(tar.pos).corps.size() >= 1) {
                getCorps(block(tar.pos).corps[0], tower.ownerID);
            }
        }
    }
    else {
        assert(0);
    }
}
// 兵团相关命令
void Game::corpsMove(CorpsInfo &corps, const vector<int> &par) {
    const static TPoint mp[4] = {{0, -1}, {0, 1}, {-1, 0}, {0, 1}};
    TPoint              pos = corps.pos, dest = corps.pos + mp[par[2]];
    if(!isPosValid(dest)) {
        return;
    }
    if(block(dest).TowerIndex != -1 &&
        !isMyTower(corps.owner, block(dest).TowerIndex)) {
        return;
    }
    if(block(dest).TowerIndex == -1 && block(dest).corps.size() >= 2) {
        return;
    }
    if(block(dest).TowerIndex == -1 && block(dest).corps.size() == 1 &&
        (!isMyCorps(corps.owner, block(dest).corps[0]) ||
        corpsInfo[block(dest).corps[0]].type == corps.type)) {
        return;
    }
    if(needMoveCost(pos, dest) > corps.movePoint) {
        return;
    }

    corps.pos = dest;
    for(size_t i = 0; i < block(pos).corps.size(); i++) {
        if(block(pos).corps[i] == corps.ID) {
            block(pos).corps.erase(block(pos).corps.begin() + i);
            break;
        }
    }
    block(dest).corps.push_back(corps.ID);
    corps.movePoint -= needMoveCost(pos, dest);
}
void Game::corpsAttackCorps(CorpsInfo &corps, const vector<int> &par) {
    TCorpsID tarID = par[2];
    if(corps.type == Construct) {
        return;
    }
    if(tarID >= int(corpsInfo.size())) {
        return;
    }
    CorpsInfo &tar = corpsInfo[tarID];
    if(!tar.exist || isMyCorps(corps.owner, tarID)) {
        return;
    }
    if(corps.movePoint == 0) {
        return;
    }
    if(getDist(corps.pos, tar.pos) > TBattleRange[corps.m_BattleType]) {
        return;
    }
    if(block(tar.pos).TowerIndex != -1) {
        corpsAttackTower(
            corps, {CAttackTower, corps.ID, block(tar.pos).TowerIndex});
        return;
    }
    if(tar.type == Construct) {
        if(block(tar.pos).corps.size() > 1) {
            int real_tar = int(block(tar.pos).corps[0] != corps.ID);
            real_tar     = block(tar.pos).corps[real_tar];
            corpsAttackCorps(corps, {CAttackCorps, corps.ID, real_tar});
        }
        else {
            getCorps(tarID, corps.owner);
        }
    }
    else if(tar.type == Battle) {
        TDoublePara fa = getCorpsForce(corps), fb = getCorpsForce(tar);
        TDoublePara myHealthLost  = (corps.m_BattleType == Archer ?
                                            0 :
                                            (28 * exp(0.04 * (fb - fa)))),
                    tarHealthLost = (30 * exp(0.04 * (fa - fb)));
        corps.healthPoint -= myHealthLost;
        if(corps.healthPoint < 0) {
            deadCorps(corps.ID);
        }
        tar.healthPoint -= tarHealthLost;
        if(tar.healthPoint < 0) {
            deadCorps(tar.ID);
            if(block(tar.pos).corps.size() >= 1 && corps.exist) {
                getCorps(block(tar.pos).corps[0], corps.owner);
            }
        }
        corps.movePoint = 0;
    }
    else {
        assert(0);
    }
}
void Game::corpsAttackTower(CorpsInfo &corps, const vector<int> &par) {
    TTowerID tarID = par[2];
    if(corps.type == Construct) {
        return;
    }
    if(tarID >= int(towerInfo.size())) {
        return;
    }
    TowerInfo &tar = towerInfo[tarID];
    if(!tar.exist || isMyTower(corps.owner, tarID)) {
        return;
    }
    if(getDist(corps.pos, tar.pos) > TBattleRange[corps.m_BattleType]) {
        return;
    }
    TDoublePara fa = getCorpsForce(corps), fb = getTowerForce(tar);
    TDoublePara myHealthLost = (corps.m_BattleType == Archer ?
                                    0 :
                                    (28 * exp(0.04 * (fb - fa)))),
                tarHealthLost =
                    (30 * corpsAttackTowerGain[corps.m_BattleType][0] *
                        exp(0.04 * (fa - fb)));
    corps.healthPoint -= myHealthLost;
    if(corps.healthPoint < 0) {
        deadCorps(corps.ID);
    }

    tar.healthPoint -= tarHealthLost;
    if(tar.healthPoint < 0) {
        if(tar.level - 4 <= 0) {
            deadTower(tar.ID);
        }
        else {
            if(corps.exist) {
                getTower(tar.ID, corps.owner);
            }
        }
    }

    corps.movePoint = 0;
}
void Game::corpsBuild(CorpsInfo &corps, const vector<int> &par) {
    if(corps.type != Construct || corps.m_BuildType != Explorer) {
        return;
    }
    if(corps.movePoint <= 0) {
        return;
    }
    if(block(corps.pos).TowerIndex != -1) {
        return;
    }
    if(!isMyCell(corps.owner, corps.pos)) {
        return;
    }
    deadCorps(corps.ID);
    addTower(corps.owner, corps.pos);
}
void Game::corpsRepair(CorpsInfo &corps, const vector<int> &par) {
    if(corps.type != Construct || corps.m_BuildType != Builder) {
        return;
    }
    if(corps.movePoint <= 0) {
        return;
    }
    TTowerID tid = block(corps.pos).TowerIndex;
    if(tid == -1 || !isMyTower(corps.owner, tid)) {
        return;
    }
    TowerInfo &  tower = towerInfo[tid];
    THealthPoint maxHealth =
        TowerInitConfig[tower.level - 1].initHealthPoint;
    tower.healthPoint =
        max(tower.healthPoint + int(double(1) / 3 * maxHealth), maxHealth);
    corps.BuildPoint -= 1;
    if(corps.BuildPoint <= 0) {
        deadCorps(corps.ID);
    }
}
void Game::corpsChangeTerrain(CorpsInfo &corps, const vector<int> &par) {
    if(corps.type != Construct || corps.m_BuildType != Builder) {
        return;
    }
    if(corps.movePoint <= 0) {
        return;
    }
    if(block(corps.pos).type == TRPlain) {
        _gameMap_backup[corps.pos.m_y][corps.pos.m_x].type = TRForest;
    }
    else if(block(corps.pos).type == TRForest) {
        _gameMap_backup[corps.pos.m_y][corps.pos.m_x].type = TRPlain;
    }
    corps.BuildPoint -= 1;
    if(corps.BuildPoint <= 0) {
        deadCorps(corps.ID);
    }
}
// 单条命令执行
void Game::doCommand(TPlayerID playerID, Command &todoCmd) {
    if(todoCmd.cmdType == towerCommand) {
        // 判断合法性
        TTowerID towerID = todoCmd.parameters[1];
        if(towerID >= (int)towerInfo.size())
            return;
        TowerInfo &tower = towerInfo[towerID];
        if(!tower.exist)
            return;
        if(!isMyTower(playerID, tower.ID))
            return;
        if(tower.haveDoneCommand == true) {
            return;
        }
        switch(todoCmd.parameters[0]) {
            case TProduct:
                addtowerProduct(tower, todoCmd.parameters);
                break;
            case TAttackCorps:
                towerAttackCorps(tower, todoCmd.parameters);
                break;
            default: assert(0); break;
        }
    }
    else if(todoCmd.cmdType == corpsCommand) {
        TCorpsID corpsID = todoCmd.parameters[1];
        if(corpsID >= (int)corpsInfo.size())
            return;
        CorpsInfo &corps = corpsInfo[corpsID];
        if(!corps.exist)
            return;
        if(!isMyCorps(playerID, corps.ID))
            return;

        switch(todoCmd.parameters[0]) {
            case CMove: corpsMove(corps, todoCmd.parameters); break;
            case CAttackCorps:
                corpsAttackCorps(corps, todoCmd.parameters);
                break;
            case CAttackTower:
                corpsAttackTower(corps, todoCmd.parameters);
                break;
            case CBuild: corpsBuild(corps, todoCmd.parameters); break;
            case CRepair: corpsRepair(corps, todoCmd.parameters); break;
            case CChangeTerrain:
                corpsChangeTerrain(corps, todoCmd.parameters);
                break;
            default: assert(0); break;
        }
    }
    else {
        assert(0);
    }
}
// 为第 id 用户执行 todoCommandList, 更新到 Info 当中
void Game::operateCommandList(TPlayerID playerID, CommandList &todoCommandList) {
    for(Command todoCommand : todoCommandList) {
        doCommand(playerID, todoCommand);
    }
}

// 维护相关函数
void Game::updateProduct(TPlayerID pid) {
    for(TowerInfo &tower : towerInfo) {
        if(!isMyTower(pid, tower.ID))
            continue;
        if(!tower.exist)
            continue;
        towerProduct(tower);
    }
}
// 维护地形。
void Game::updateTerrain() {
    for(int j = 0; j < m_width; j++) {
        for(int i = 0; i < m_height; i++) {
            if(gameMap[j][i].type != TRTower) {
                gameMap[j][i].type = _gameMap_backup[j][i].type;
            }
        }
    }
}
// 更新 格子的归属。 全图更新，先都抹掉，再重新计算
void Game::updateMapOwner() {
    // todo: 别忘了加过渡的单位
    for(int j = 0; j < m_width; j++) {
        for(int i = 0; i < m_height; i++) {
            block({i, j}).occupyPoint.resize(0);
            block({i, j}).occupyPoint.resize(totalPlayers);
            for(int pid = 1; pid <= totalPlayers; pid++)
                block({i, j}).occupyPoint[pid - 1] = 0;
        }
    }
    for(TowerInfo tower : towerInfo) {
        if(!tower.exist)
            continue;
        TPoint    pos     = tower.pos;
        TPlayerID ownerID = tower.ownerID;

        int L = OCCUPY_POINT_DIST_SCALE;
        for(int j = -L; j <= L; j++) {
            for(int i = -L; i <= L; i++) {
                TPoint t = pos + TPoint({i, j});
                if(!isPosValid(t))
                    continue;
                block(t).occupyPoint[ownerID - 1] +=
                    TowerOccupyPoint[getDist(pos, t)];
            }
        }
    }
    for(int j = 0; j < m_width; j++) {
        for(int i = 0; i < m_height; i++) {
            int       maxPoint = 0, maxPlayer = -INF, draw = 0;
            mapBlock &cell = block({i, j});
            for(int pid = 1; pid <= totalPlayers; pid++) {
                if(cell.occupyPoint[pid - 1] > maxPoint) {
                    maxPoint  = cell.occupyPoint[pid - 1];
                    maxPlayer = pid;
                    draw      = 0;
                }
                else if(cell.occupyPoint[pid - 1] == maxPoint) {
                    draw = 1;
                }
            }
            if(maxPoint == 0) {
                cell.owner = PUBLIC;
            }
            else {
                if(draw == 1) {
                    cell.owner = TRANSITION;
                }
                else {
                    cell.owner = maxPlayer;
                }
            }
        }
    }
}
// 更新玩家占领地盘和分数，判断玩家是否出局
void Game::updatePlayer() {
    playerAlive = 0;
    for(TPlayerID pid = 1; pid <= totalPlayers; pid++) {
        if(playerInfo[pid - 1].tower.size() == 0) {
            playerInfo[pid - 1].score =
                (-totalPlayers * MAX_ROUND + totalRounds - 100) / 100;
            playerInfo[pid - 1].alive = false;
        }
        else {
            playerInfo[pid - 1].alive = true;
            playerAlive++;
        }
        if(playerInfo[pid - 1].score > 0) {
            playerInfo[pid - 1].score = 0;
        }
    }
    for(int j = 0; j < m_width; j++) {
        for(int i = 0; i < m_height; i++) {
            TPlayerID owner = block({i, j}).owner;
            if(owner >= 1 && owner <= 4)
                playerInfo[owner - 1].score++;
        }
    }
}
// 回合之后统一的update
void Game::updateInfo() {
    // lots of things to do .
    updateTerrain();
    updatePlayer();
}
// 刷新兵团建设力和行动力
void Game::refresh() {
    for(CorpsInfo &_c : corpsInfo) {
        _c.refresh();
    }
    for(TowerInfo &_t : towerInfo) {
        _t.refresh();
    }
}
Json::Value Game::play(TPlayerID playerID, CommandList &todoCommandList) {
    refresh();  // 全局刷新，不是你的也刷新了
    operateCommandList(playerID, todoCommandList);
    updateProduct(playerID);
    updateInfo();
    return asJson();
}
void Game::print() {
    for(int i = 0; i < m_height; i++) {
        for(int j = 0; j < m_width; j++) {
            string output;
            if(block({i, j}).owner == 1) {
                output = L_RED;
            }
            if(block({i, j}).owner == 2) {
                output = L_GREEN;
            }
            if(block({i, j}).owner == 3) {
                output = L_BLUE;
            }
            if(block({i, j}).owner == 4) {
                output = L_PURPLE;
            }
            output = output + "%4d" + NONE;
            printf(output.c_str(), block({i, j}).TowerIndex);
        }
        printf("\n");
    }
    printf("\nPress enter to continue");
    getchar();
}