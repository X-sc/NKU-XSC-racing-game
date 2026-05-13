#pragma once
#include "raylib.h"
#include <stdio.h>
#include <string.h>

#define SCREEN_WIDTH  1400
#define SCREEN_HEIGHT 900
#define ROAD_X       460
#define ROAD_WIDTH   480
#define LANE_COUNT   3
#define MAX_HP 3
#define CAR_COUNT 3
#define MAX_OBSTACLE 8
#define MAX_ITEM     3

typedef enum
{
    STATE_SELECT_CAR,
    STATE_COUNTDOWN,
    STATE_PLAYING,
    STATE_OVER
} GameState;

typedef enum
{
    OBJ_CAT,
    OBJ_PEOPLE,
    OBJ_BIKE,
    OBJ_BUS,
    OBJ_STATIC
} ObstacleType;

typedef enum
{
    ITEM_HEART,
    ITEM_STAR,
    ITEM_SPEED
} ItemType;

typedef struct
{
    float x, y;
    int lane;
    int hp;
    int carId;
} Player;

typedef struct
{
    float x, y, w, h;
    float spdY, spdX;
    ObstacleType type;
    bool active;
} Obstacle;

typedef struct
{
    float x, y, w, h;
    float spdY;
    ItemType type;
    bool active;
} Item;

typedef struct
{
    char text[32];
    Color color;
    float timer;
} PopupTip;

extern Texture2D carTextures[CAR_COUNT];
extern PopupTip g_popup;