#pragma once
#include "raylib.h"

// 窗口
#define SCREEN_WIDTH  1400
#define SCREEN_HEIGHT 900

// 马路
#define ROAD_X       460
#define ROAD_WIDTH   480
#define LANE_COUNT   3

// 游戏最大血量
#define MAX_HP 3

// 游戏状态
typedef enum
{
    STATE_MENU,
    STATE_PLAY,
    STATE_OVER
} GameState;

// 障碍物类型：小猫 / 行人 / 自行车 / 公交车 / 静止障碍
typedef enum
{
    OBJ_CAT,
    OBJ_PEOPLE,
    OBJ_BIKE,
    OBJ_BUS,
    OBJ_STATIC
} ObstacleType;

// 道具类型
typedef enum
{
    ITEM_HEART,
    ITEM_STAR,
    ITEM_SPEED
} ItemType;

// 玩家
typedef struct
{
    float x, y;
    int lane;
    int hp;
} Player;

// 单个障碍物
typedef struct
{
    float x, y, w, h;
    float spdY, spdX;
    ObstacleType type;
    bool active;
} Obstacle;

// 单个道具
typedef struct
{
    float x, y, w, h;
    float spdY;
    ItemType type;
    bool active;
} Item;

// 全局最大数量
#define MAX_OBSTACLE 8
#define MAX_ITEM     3