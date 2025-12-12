#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define CARD_COUNT 8      // 卡牌种类数量
#define MAX_HAND 4        // 手牌数量
#define MAX_DECK 8        // 卡组数量
#define PLAYER_ELIXIR 10  // 玩家最大圣水量
#define COMPUTER_ELIXIR 10 // 电脑最大圣水量
#define BATTLEFIELD_WIDTH 9 // 战场宽度

// 卡牌结构体
typedef struct {
    char name[20];      // 卡牌名称
    int cost;          // 圣水消耗
    int damage;        // 攻击力
    int health;        // 生命值
    int range;         // 攻击范围
    char type;         // 类型: 'G'地面, 'A'空中, 'B'建筑
    char symbol;       // 显示符号
} Card;

// 战场单位结构体
typedef struct {
    Card card;         // 对应的卡牌
    int x;             // 位置
    int y;             // 阵营: 1=玩家, 2=电脑
    int currentHealth; // 当前生命值
    int cooldown;      // 攻击冷却
} BattleUnit;

// 玩家结构体
typedef struct {
    int elixir;                   // 当前圣水
    int elixirTimer;              // 圣水恢复计时器
    int towerHealth;              // 公主塔生命值
    int kingTowerHealth;          // 国王塔生命值
    Card deck[MAX_DECK];          // 卡组
    Card hand[MAX_HAND];          // 手牌
    int handCount;                // 手牌数量
    BattleUnit units[20];         // 战场上的单位
    int unitCount;                // 单位数量
} Player;

// 全局变量
Card allCards[CARD_COUNT];        // 所有可用卡牌
Player player;                    // 玩家
Player computer;                  // 电脑
BattleUnit battlefield[5][BATTLEFIELD_WIDTH]; // 战场网格
int gameOver = 0;                 // 游戏结束标志
int turn = 0;                     // 回合数

// 初始化卡牌
void initCards() {
    // 骑士
    strncpy(allCards[0].name, "骑士", sizeof(allCards[0].name) - 1);
    allCards[0].name[sizeof(allCards[0].name) - 1] = '\0';
    allCards[0].cost = 3;
    allCards[0].damage = 100;
    allCards[0].health = 1500;
    allCards[0].range = 1;
    allCards[0].type = 'G';
    allCards[0].symbol = 'K';
    
    // 弓箭手
    strncpy(allCards[1].name, "弓箭手", sizeof(allCards[1].name) - 1);
    allCards[1].name[sizeof(allCards[1].name) - 1] = '\0';
    allCards[1].cost = 3;
    allCards[1].damage = 50;
    allCards[1].health = 200;
    allCards[1].range = 5;
    allCards[1].type = 'G';
    allCards[1].symbol = 'A';
    
    // 巨人
    strncpy(allCards[2].name, "巨人", sizeof(allCards[2].name) - 1);
    allCards[2].name[sizeof(allCards[2].name) - 1] = '\0';
    allCards[2].cost = 5;
    allCards[2].damage = 150;
    allCards[2].health = 3000;
    allCards[2].range = 1;
    allCards[2].type = 'G';
    allCards[2].symbol = 'G';
    
    // 火球
    strncpy(allCards[3].name, "火球", sizeof(allCards[3].name) - 1);
    allCards[3].name[sizeof(allCards[3].name) - 1] = '\0';
    allCards[3].cost = 4;
    allCards[3].damage = 300;
    allCards[3].health = 0;
    allCards[3].range = 3;
    allCards[3].type = 'B';
    allCards[3].symbol = 'F';
    
    // 亡灵
    strncpy(allCards[4].name, "亡灵", sizeof(allCards[4].name) - 1);
    allCards[4].name[sizeof(allCards[4].name) - 1] = '\0';
    allCards[4].cost = 3;
    allCards[4].damage = 80;
    allCards[4].health = 100;
    allCards[4].range = 1;
    allCards[4].type = 'A';
    allCards[4].symbol = 'S';
    
    // 加农炮
    strncpy(allCards[5].name, "加农炮", sizeof(allCards[5].name) - 1);
    allCards[5].name[sizeof(allCards[5].name) - 1] = '\0';
    allCards[5].cost = 3;
    allCards[5].damage = 120;
    allCards[5].health = 800;
    allCards[5].range = 5;
    allCards[5].type = 'B';
    allCards[5].symbol = 'C';
    
    // 王子
    strncpy(allCards[6].name, "王子", sizeof(allCards[6].name) - 1);
    allCards[6].name[sizeof(allCards[6].name) - 1] = '\0';
    allCards[6].cost = 5;
    allCards[6].damage = 200;
    allCards[6].health = 1800;
    allCards[6].range = 1;
    allCards[6].type = 'G';
    allCards[6].symbol = 'P';
    
    // 飞龙宝宝
    strncpy(allCards[7].name, "飞龙宝宝", sizeof(allCards[7].name) - 1);
    allCards[7].name[sizeof(allCards[7].name) - 1] = '\0';
    allCards[7].cost = 4;
    allCards[7].damage = 80;
    allCards[7].health = 1000;
    allCards[7].range = 3;
    allCards[7].type = 'A';
    allCards[7].symbol = 'D';
}

// 初始化玩家
void initPlayer(Player *p, int isComputer) {
    p->elixir = isComputer ? 4 : 5; // 电脑初始圣水少一些
    p->elixirTimer = 0;
    p->towerHealth = 2400;
    p->kingTowerHealth = 4800;
    p->handCount = 0;
    p->unitCount = 0;
    
    // 随机选择卡组
    for (int i = 0; i < MAX_DECK; i++) {
        p->deck[i] = allCards[rand() % CARD_COUNT];
    }
    
    // 初始手牌
    for (int i = 0; i < MAX_HAND; i++) {
        p->hand[i] = p->deck[i];
        p->handCount++;
    }
}

// 初始化战场
void initBattlefield() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < BATTLEFIELD_WIDTH; j++) {
            battlefield[i][j].card.name[0] = '\0';
            battlefield[i][j].x = j;
            battlefield[i][j].y = i;
        }
    }
}

// 显示游戏界面
void displayGame() {
    system("cls"); // 清屏
    
    printf("======================= 部落冲突：皇室战争 =======================\n");
    printf("回合: %d\n", turn);
    printf("\n");
    
    // 显示电脑信息
    printf("电脑 圣水: %d/%d  公主塔: %d  国王塔: %d\n", 
           computer.elixir, COMPUTER_ELIXIR, computer.towerHealth, computer.kingTowerHealth);
    
    // 显示战场
    printf("\n");
    printf("  ");
    for (int i = 0; i < BATTLEFIELD_WIDTH; i++) {
        printf("%d ", i);
    }
    printf("\n");
    
    for (int i = 0; i < 5; i++) {
        printf("%d ", i);
        for (int j = 0; j < BATTLEFIELD_WIDTH; j++) {
            if (battlefield[i][j].card.name[0] != '\0') {
                // 根据阵营显示不同颜色
                if (battlefield[i][j].y == 0) {
                    printf("\033[34m%c\033[0m ", battlefield[i][j].card.symbol); // 蓝色-玩家
                } else if (battlefield[i][j].y == 4) {
                    printf("\033[31m%c\033[0m ", battlefield[i][j].card.symbol); // 红色-电脑
                } else {
                    printf("%c ", battlefield[i][j].card.symbol);
                }
            } else {
                if (i == 0 || i == 4) {
                    printf("▓ "); // 塔的位置
                } else {
                    printf(". ");
                }
            }
        }
        printf("\n");
    }
    
    // 显示玩家信息
    printf("\n玩家 圣水: %d/%d  公主塔: %d  国王塔: %d\n", 
           player.elixir, PLAYER_ELIXIR, player.towerHealth, player.kingTowerHealth);
    
    // 显示手牌
    printf("\n你的手牌:\n");
    for (int i = 0; i < player.handCount; i++) {
        printf("%d. %s (消耗:%d 伤害:%d 生命:%d 类型:%c)\n", 
               i+1, player.hand[i].name, player.hand[i].cost, 
               player.hand[i].damage, player.hand[i].health, player.hand[i].type);
    }
    
    printf("\n命令: p=放置卡牌, s=跳过, q=退出\n");
}

// 放置单位到战场
int placeUnit(Player *p, int cardIndex, int x, int y) {
    if (cardIndex < 0 || cardIndex >= p->handCount) {
        return 0; // 无效的卡牌索引
    }
    
    Card selectedCard = p->hand[cardIndex];
    
    // 检查圣水是否足够
    if (p->elixir < selectedCard.cost) {
        printf("圣水不足！需要%d，当前只有%d\n", selectedCard.cost, p->elixir);
        return 0;
    }
    
    // 根据玩家决定行阵营（不再忽略参数 y）
    int actualY = (p == &player) ? 0 : 4;
    
    // 检查位置是否有效
    if (x < 0 || x >= BATTLEFIELD_WIDTH || actualY < 0 || actualY >= 5) {
        printf("无效的位置！\n");
        return 0;
    }
    
    // 检查位置是否被占用
    if (battlefield[actualY][x].card.name[0] != '\0') {
        printf("该位置已被占用！\n");
        return 0;
    }
    
    // 创建战场单位
    BattleUnit newUnit;
    newUnit.card = selectedCard;
    newUnit.x = x;
    newUnit.y = actualY;
    newUnit.currentHealth = selectedCard.health;
    newUnit.cooldown = 0;
    
    // 添加到战场
    battlefield[actualY][x] = newUnit;
    
    // 添加到玩家单位列表
    p->units[p->unitCount] = newUnit;
    p->unitCount++;
    
    // 消耗圣水
    p->elixir -= selectedCard.cost;
    
    // 从手牌中移除
    for (int i = cardIndex; i < p->handCount - 1; i++) {
        p->hand[i] = p->hand[i + 1];
    }
    p->handCount--;
    
    // 从卡组中抽取新卡牌
    if (p->handCount < MAX_HAND) {
        int deckIndex = rand() % MAX_DECK;
        p->hand[p->handCount] = p->deck[deckIndex];
        p->handCount++;
    }
    
    return 1;
}

// 电脑AI
void computerTurn() {
    // 简单AI：随机放置卡牌
    if (computer.handCount > 0 && computer.elixir >= 2) {
        int attempts = 0;
        while (attempts < 10) {
            int cardIndex = rand() % computer.handCount;
            int x = rand() % BATTLEFIELD_WIDTH;
            int y = 4; // 电脑在顶部
            
            if (computer.elixir >= computer.hand[cardIndex].cost && 
                battlefield[y][x].card.name[0] == '\0') {
                printf("电脑放置了 %s 在位置(%d,%d)\n", 
                       computer.hand[cardIndex].name, x, y);
                placeUnit(&computer, cardIndex, x, y);
                break;
            }
            attempts++;
        }
    }
}

// 单位行动
void unitActions() {
    // 移动和攻击
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < BATTLEFIELD_WIDTH; j++) {
            if (battlefield[i][j].card.name[0] != '\0') {
                BattleUnit *unit = &battlefield[i][j];
                
                // 减少攻击冷却
                if (unit->cooldown > 0) {
                    unit->cooldown--;
                }
                
                // 寻找目标
                int targetX = -1, targetY = -1;
                int minDistance = 100;
                
                for (int y = 0; y < 5; y++) {
                    for (int x = 0; x < BATTLEFIELD_WIDTH; x++) {
                        if (battlefield[y][x].card.name[0] != '\0') {
                            // 检查是否是敌人
                            int isEnemy = (unit->y == 0 && y == 4) || 
                                         (unit->y == 4 && y == 0) ||
                                         (unit->y == 0 && y > 0 && y < 4) ||
                                         (unit->y == 4 && y > 0 && y < 4);
                            
                            if (isEnemy) {
                                int distance = abs(x - unit->x) + abs(y - unit->y);
                                if (distance < minDistance && distance <= unit->card.range) {
                                    minDistance = distance;
                                    targetX = x;
                                    targetY = y;
                                }
                            }
                        }
                    }
                }
                
                // 如果找到目标且冷却为0，攻击
                if (targetX != -1 && unit->cooldown == 0) {
                    BattleUnit *target = &battlefield[targetY][targetX];
                    target->currentHealth -= unit->card.damage;
                    unit->cooldown = 2; // 攻击后进入冷却
                    
                    printf("%s(%d,%d) 攻击了 %s(%d,%d) 造成%d伤害\n",
                           unit->card.name, unit->x, unit->y,
                           target->card.name, target->x, target->y,
                           unit->card.damage);
                    
                    // 如果目标生命值<=0，从战场移除
                    if (target->currentHealth <= 0) {
                        printf("%s(%d,%d) 被击败\n", target->card.name, target->x, target->y);
                        target->card.name[0] = '\0';
                    }
                }
                // 如果没有目标在范围内，移动
                else if (targetX == -1) {
                    // 根据阵营决定移动方向
                    int moveDir = (unit->y == 0) ? 1 : -1; // 玩家向上，电脑向下
                    
                    // 检查前方是否为空
                    int newY = unit->y + moveDir;
                    if (newY >= 0 && newY < 5 && 
                        battlefield[newY][unit->x].card.name[0] == '\0') {
                        // 移动单位
                        battlefield[newY][unit->x] = *unit;
                        battlefield[unit->y][unit->x].card.name[0] = '\0';
                        // 更新位置（重新获取指针以避免悬空指针）
                        battlefield[newY][unit->x].y = newY;
                    }
                }
            }
        }
    }
    
    // 检查是否有单位到达对方塔
    for (int x = 0; x < BATTLEFIELD_WIDTH; x++) {
        // 玩家单位到达电脑塔
        if (battlefield[4][x].card.name[0] != '\0' && battlefield[4][x].y == 0) {
            computer.towerHealth -= battlefield[4][x].card.damage * 2;
            printf("玩家单位攻击电脑公主塔，造成%d伤害\n", battlefield[4][x].card.damage * 2);
            battlefield[4][x].card.name[0] = '\0';
        }
        
        // 电脑单位到达玩家塔
        if (battlefield[0][x].card.name[0] != '\0' && battlefield[0][x].y == 4) {
            player.towerHealth -= battlefield[0][x].card.damage * 2;
            printf("电脑单位攻击玩家公主塔，造成%d伤害\n", battlefield[0][x].card.damage * 2);
            battlefield[0][x].card.name[0] = '\0';
        }
    }
}

// 更新圣水
void updateElixir() {
    // 玩家圣水恢复
    player.elixirTimer++;
    if (player.elixirTimer >= 3 && player.elixir < PLAYER_ELIXIR) {
        player.elixir++;
        player.elixirTimer = 0;
    }
    
    // 电脑圣水恢复
    computer.elixirTimer++;
    if (computer.elixirTimer >= 3 && computer.elixir < COMPUTER_ELIXIR) {
        computer.elixir++;
        computer.elixirTimer = 0;
    }
}

// 检查游戏状态
void checkGameState() {
    if (player.towerHealth <= 0 || player.kingTowerHealth <= 0) {
        printf("电脑获胜！\n");
        gameOver = 1;
    } else if (computer.towerHealth <= 0 || computer.kingTowerHealth <= 0) {
        printf("玩家获胜！\n");
        gameOver = 1;
    } else if (turn >= 100) {
        printf("平局！回合数达到上限\n");
        gameOver = 1;
    }
}

// 主游戏循环
void gameLoop() {
    char command;
    int cardIndex, x, y;
    
    while (!gameOver) {
        displayGame();
        
        // 玩家回合
        printf("你的行动: ");
        scanf(" %c", &command);
        
        if (command == 'q') {
            printf("游戏结束\n");
            break;
        } else if (command == 'p') {
            printf("选择卡牌(1-%d): ", player.handCount);
            scanf("%d", &cardIndex);
            printf("选择位置(x y): ");
            scanf("%d %d", &x, &y);
            
            if (placeUnit(&player, cardIndex-1, x, y)) {
                printf("成功放置 %s\n", player.hand[cardIndex-1].name);
            }
        } else if (command == 's') {
            printf("跳过回合\n");
        }
        
        // 电脑回合
        computerTurn();
        
        // 单位行动
        unitActions();
        
        // 更新圣水
        updateElixir();
        
        // 检查游戏状态
        checkGameState();
        
        turn++;
        
        // 暂停一下
        Sleep(1000);
    }
}

// 主函数
int main() {
    srand(time(NULL)); // 随机种子
    
    printf("欢迎来到简化版部落冲突：皇室战争！\n");
    printf("游戏规则:\n");
    printf("1. 使用圣水放置卡牌\n");
    printf("2. 单位会自动移动和攻击\n");
    printf("3. 摧毁对方公主塔获胜\n");
    printf("4. 国王塔在公主塔被摧毁后激活\n");
    printf("按回车键开始游戏...\n");
    getchar();
    
    // 初始化游戏
    initCards();
    initPlayer(&player, 0);
    initPlayer(&computer, 1);
    initBattlefield();
    
    // 开始游戏
    gameLoop();
    
    printf("感谢游玩！\n");
    
    return 0;
}

