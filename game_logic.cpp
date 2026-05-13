#include "game_def.h"
#include <math.h>
#include <cstring>

// ====================== 宏定义 ======================
// 难度选择状态
#ifndef STATE_DIFFICULTY
#define STATE_DIFFICULTY -1
#endif

// 天气系统
#define WEATHER_MIN_INTERVAL 10800.0f
#define WEATHER_MAX_INTERVAL 18000.0f
#define WEATHER_TRANSITION_TIME 120.0f

// 水坑系统
#define PUDDLE_MAX 8
#define SLIP_DURATION 60.0f

//太阳眩光系统
#define GLARE_DURATION 120.0f
#define GLARE_INTERVAL_MIN 1800.0f
#define GLARE_INTERVAL_MAX 3600.0f

// 幽灵障碍物系统
#define GHOST_OBSTACLE_MAX 5
#define GHOST_SPAWN_INTERVAL 150.0f

// 冰雹系统
#define HAIL_MAX 30
#define RAIN_MAX 120

// 赛道事件系统
#define ROADBLOCK_MAX 3
#define ROADBLOCK_INTERVAL 2400.0f
#define SPEEDBELT_MAX 2
#define SPEEDBELT_INTERVAL 1500.0f
#define BELT_SPEED_DURATION 120.0f

// 氮气 & 连击系统
#define ITEM_LIFE 0    // 生命道具 对应贴图 life.png
#define ITEM_SCORE 1   // 分数道具 对应贴图 score.png
#define ITEM_SPEED 2   // 加速道具 对应贴图 speed.png
#define ITEM_SHIELD 3  // 护盾道具 对应贴图 shield.png
#define ITEM_SLOW 4    // 减速道具 对应贴图 slow.png
#define ITEM_NITRO 5   // 氮气道具 对应贴图 nitro.png
#define ITEM_MAGNET 6  // 磁铁道具 对应贴图 magnet.png
#define NITRO_MAX 100.0f
#define NITRO_BURST_MUL 4.5f
#define NITRO_SUSTAIN_MUL 3.5f
#define NITRO_BURST_TIME 60.0f
#define NITRO_CONSUME_RATE 0.4f
#define NITRO_CAMERA_ZOOM 1.05f
#define COMBO_TIMEOUT 120.0f
#define MAGNET_DURATION 600.0f

// 游戏核心宏
#define HIGHSCORE_FILE "highscore.dat"
#define PLAYER_BASE_SPEED 5.0f
#define PLAYER_SPEED_MAX  8.5f
#define SPEED_BUFF_DURATION 180.0f
#define SHIELD_DURATION 300.0f
#define SLOW_DURATION 240.0f
#define SPD_BIKE    7.2f
#define SPD_CAT     6.5f
#define SPD_BUS     5.8f
#define SPD_PEOPLE  5.0f
#define SPD_STATIC  4.5f
#define STATIC_SPAWN_INTERVAL 220
#define EXHAUST_MAX 32
#define SPEED_LINE_MAX 20

// ====================== 全局贴图缩放系数======================
#define TEXTURE_GLOBAL_SCALE 2.0f

// ====================== 背景系统宏定义 ======================
#define SPLASH_DISPLAY_TIME 180.0f  // 启动页显示3s
#define SPLASH_FADE_TIME    60.0f   // 启动页淡出1s
#define BG_FRAME_COUNT      1       
#define BG_SCROLL_SPEED     2.0f    // 基础滚动速度
#define BG_DARK_ALPHA       0.7f    // 基础背景暗化系数

// ====================== 氮气特效增强参数 ======================
#define NITRO_BG_SPEED_MUL 5.0f    // 氮气时背景速度倍数
#define NITRO_BG_DARK_MUL  0.6f    // 氮气时背景亮度比例
#define BG_TRANSITION_SPEED 8.0f   // 亮度/速度过渡速度
// ======================枚举类型定义======================
// 天气类型枚举：晴天、雨天、雾天、冰雹
typedef enum {
    WEATHER_SUNNY,
    WEATHER_RAINY,
    WEATHER_FOGGY,
    WEATHER_HAIL
} WeatherType;

// 游戏难度枚举：简单、普通、困难
typedef enum {
    DIFF_EASY,
    DIFF_NORMAL,
    DIFF_HARD
} Difficulty;

// ======================结构体定义======================
typedef struct { float x, y, w, h; bool active; bool hasItem; } Puddle;
typedef struct { float timer; bool active; } SunGlare;
typedef struct { float x, y, w, h, spdY; bool active; ObstacleType type; } GhostObstacle;
typedef struct { float x, y, spd, w, h; bool active; } Hail;
typedef struct { float x, y, w, h; int lane; bool active; } RoadBlock;
typedef struct { float x, y, w, h; bool active; } SpeedBelt;
typedef struct { float x, y, spd; } RainDrop;
typedef struct { float x, y, life; Color col; } Exhaust;
typedef struct { float x, y, spd; } SpeedLine;

// ====================== 全局贴图变量 ======================
Texture2D carTextures[CAR_COUNT];          // 3辆玩家车
Texture2D obstacleTextures[5];             // 5种障碍物
Texture2D itemTextures[7];                 // 7种道具

// 特殊元素贴图 
Texture2D ghostTexture;     // 雾天幽灵障碍物
Texture2D puddleTexture;    // 雨天水坑
Texture2D roadblockTexture; // 三连路障
Texture2D glareTexture;     // 太阳眩光

// ====================== 背景系统贴图======================
Texture2D g_splashTex;      // 程序启动页
Texture2D g_menuBgTex;      // 难度选择+选车页
Texture2D g_sunBg[BG_FRAME_COUNT];  // 晴天
Texture2D g_fogBg[BG_FRAME_COUNT];  // 雾天
Texture2D g_rainBg[BG_FRAME_COUNT]; // 雨天/冰雹

// ====================== 全局游戏变量 ======================
int g_currentWeather = WEATHER_SUNNY;
int g_nextWeather = WEATHER_SUNNY;
bool g_isWeatherChanging = false;
float g_weatherTimer = 0.0f;
float g_weatherTransition = 0.0f;
float g_rainAlpha = 0.0f, g_fogAlpha = 0.0f, g_hailAlpha = 0.0f;

Puddle g_puddleList[PUDDLE_MAX];
float g_slipTimer = 0.0f;
SunGlare g_sunGlare = { 0 };
float g_glareIntervalTimer = 0.0f;
GhostObstacle g_ghostObsList[GHOST_OBSTACLE_MAX];
float g_ghostSpawnTimer = 0.0f;
Hail g_hailList[HAIL_MAX];
RoadBlock g_roadBlockList[ROADBLOCK_MAX];
float g_roadBlockIntervalTimer = 0.0f;
SpeedBelt g_speedBeltList[SPEEDBELT_MAX];
float g_speedBeltIntervalTimer = 0.0f;
float g_beltSpeedBuffTimer = 0.0f;
RainDrop g_rainList[RAIN_MAX];

float g_nitroValue = 0.0f;
bool g_isNitroActive = false;
float g_nitroBurstTimer = 0.0f;
int g_comboCount = 0;
float g_comboTimer = 0.0f;
float g_magnetTimer = 0.0f;

int g_selectedDifficulty = DIFF_NORMAL;
int g_highScore[3] = { 0, 0, 0 };
PopupTip g_popup = { 0 };
int       g_score = 0;
float     g_flashTimer = 0.0f;
int       g_selectedCar = 0;
float     g_countdownTimer = 0.0f;
int       g_countdownNum = 3;
int       g_spawnTimer = 0;
int       g_spawnGap = 100;
int       g_staticSpawnTimer = 0;
float     g_obsBaseSpd = 5.0f;
float     g_laneWidth;
float     g_laneCenter[LANE_COUNT];
Player    g_player;
Obstacle  g_obsList[MAX_OBSTACLE];
Item      g_itemList[MAX_ITEM];

float g_speedBuffTimer = 0.0f;
float g_shieldTimer = 0.0f;
float g_slowTimer = 0.0f;
bool g_isPause = false;
float g_shakeTimer = 0.0f;
float g_shakeOffsetX = 0.0f;
float g_shakeOffsetY = 0.0f;

Exhaust g_exhaust[EXHAUST_MAX];
SpeedLine g_speedLines[SPEED_LINE_MAX];
GameState g_state = (GameState)STATE_DIFFICULTY;
float g_playerMoveVelX = 0.0f;

// ====================== 启动页+背景系统全局变量 ======================
float g_splashTimer = 0.0f;
float g_splashAlpha = 1.0f;
bool g_splashDone = false;
float g_bgOffset = 0.0f;

// 氮气特效过渡变量
float g_bgCurrentSpeedMul = 1.0f;
float g_bgCurrentDarkMul = 1.0f;

// ======================提示文字数组 ======================
const char* tipLife[] = { "HP Full!", "Nice!", "Keep Going!" };
const char* tipScore[] = { "Bonus!", "Lucky!", "Score Up!" };
const char* tipSpeed[] = { "Speed Up!", "Go Fast!", "Max Speed!" };
const char* tipHit[] = { "Ouch!", "Be Careful!", "Watch Out!" };
const char* tipShield[] = { "Shield On!", "Invincible!", "Unstoppable!" };
const char* tipSlow[] = { "Slowed!", "Slow Down!", "Careful!" };
const char* tipNitro[] = { "NITRO BOOST!", "FULL POWER!", "BLAST OFF!" };
const char* tipWeatherSunny[] = { "Sunny!", "Clear View!", "Good Weather!" };
const char* tipWeatherRainy[] = { "It's Raining!", "Slippery!", "Watch Control!" };
const char* tipWeatherFoggy[] = { "Foggy!", "Low Visibility!", "Watch Out!" };
const char* tipWeatherHail[] = { "Hail Storm!", "Watch Out!", "Break Hail!" };
const char* tipComboBreak[] = { "Watch Pedestrian!", "Pedestrian Hit!", "Slow Down!" };
const char* tipCombo10[] = { "10 COMBO!", "Road Killer!", "Nice!" };
const char* tipCombo20[] = { "20 COMBO!", "Speed Legend!", "Unstoppable!" };
const char* tipRoadBlock[] = { "Road Work!", "Change Lane!", "Watch Out!" };
const char* tipGlare[] = { "Sun Glare!", "Watch Road!", "Change Lane!" };
const char* tipMagnet[] = { "MAGNET!", "Auto Collect!", "Wow!" };

// ====================== 工具函数======================
// 显示游戏弹窗提示
static void ShowPopup(const char* text, Color color) {
    strcpy_s(g_popup.text, 32, text);
    g_popup.color = color;
    g_popup.timer = 80.0f;
}

// 触发屏幕抖动效果
static void TriggerShake() {
    g_shakeTimer = 15.0f;
}

// ====================== 物体不重叠碰撞检测 ======================
static bool IsAreaOccupied(float x, float y, float w, float h) {
    float left = x - w / 2;
    float right = x + w / 2;
    float top = y;
    float bottom = y + h;

    // 检查和普通障碍物是否重叠
    for (int i = 0; i < MAX_OBSTACLE; i++) {
        if (g_obsList[i].active) {
            float oLeft = g_obsList[i].x - g_obsList[i].w / 2;
            float oRight = g_obsList[i].x + g_obsList[i].w / 2;
            float oTop = g_obsList[i].y;
            float oBottom = g_obsList[i].y + g_obsList[i].h;

            if (!(right < oLeft || left > oRight || bottom < oTop || top > oBottom)) {
                return true;
            }
        }
    }

    // 检查和道具是否重叠
    for (int i = 0; i < MAX_ITEM; i++) {
        if (g_itemList[i].active) {
            float iLeft = g_itemList[i].x - 20;
            float iRight = g_itemList[i].x + 20;
            float iTop = g_itemList[i].y;
            float iBottom = g_itemList[i].y + 40;

            if (!(right < iLeft || left > iRight || bottom < iTop || top > iBottom)) {
                return true;
            }
        }
    }

    // 检查和水坑是否重叠
    for (int i = 0; i < PUDDLE_MAX; i++) {
        if (g_puddleList[i].active) {
            float pLeft = g_puddleList[i].x;
            float pRight = g_puddleList[i].x + g_puddleList[i].w;
            float pTop = g_puddleList[i].y;
            float pBottom = g_puddleList[i].y + g_puddleList[i].h;

            if (!(right < pLeft || left > pRight || bottom < pTop || top > pBottom)) {
                return true;
            }
        }
    }

    // 检查和路障是否重叠
    for (int i = 0; i < ROADBLOCK_MAX; i++) {
        if (g_roadBlockList[i].active) {
            float rLeft = g_roadBlockList[i].x;
            float rRight = g_roadBlockList[i].x + g_roadBlockList[i].w;
            float rTop = g_roadBlockList[i].y;
            float rBottom = g_roadBlockList[i].y + g_roadBlockList[i].h;

            if (!(right < rLeft || left > rRight || bottom < rTop || top > rBottom)) {
                return true;
            }
        }
    }

    // 检查和幽灵障碍物是否重叠
    for (int i = 0; i < GHOST_OBSTACLE_MAX; i++) {
        if (g_ghostObsList[i].active) {
            float gLeft = g_ghostObsList[i].x - g_ghostObsList[i].w / 2;
            float gRight = g_ghostObsList[i].x + g_ghostObsList[i].w / 2;
            float gTop = g_ghostObsList[i].y;
            float gBottom = g_ghostObsList[i].y + g_ghostObsList[i].h;

            if (!(right < gLeft || left > gRight || bottom < gTop || top > gBottom)) {
                return true;
            }
        }
    }

    return false;
}

// ====================== 障碍物碰撞检测与物理反弹 ======================
static void ResolveCollision(Obstacle* a, Obstacle* b) {
    // 计算两个物体的中心距离
    float dx = a->x - b->x;
    float dy = (a->y + a->h / 2) - (b->y + b->h / 2);

    // 计算最小不重叠距离
    float minDistX = (a->w + b->w) / 2;
    float minDistY = (a->h + b->h) / 2;

    // 如果没有重叠，直接返回
    if (fabsf(dx) > minDistX || fabsf(dy) > minDistY) return;

    // 计算重叠量
    float overlapX = minDistX - fabsf(dx);
    float overlapY = minDistY - fabsf(dy);

    // 沿重叠量小的方向分离
    if (overlapX < overlapY) {
        float pushX = overlapX * (dx > 0 ? 1 : -1) / 2;
        a->x += pushX;
        b->x -= pushX;
        // 水平方向反弹
        a->spdX = -a->spdX * 0.5f;
        b->spdX = -b->spdX * 0.5f;
    }
    else {
        float pushY = overlapY * (dy > 0 ? 1 : -1) / 2;
        a->y += pushY;
        b->y -= pushY;
        // 垂直方向反弹
        float avgSpd = (a->spdY + b->spdY) / 2;
        a->spdY = avgSpd + 0.5f;
        b->spdY = avgSpd - 0.5f;
    }
}

static void LoadHighScore() {
    FILE* f = NULL;
    g_highScore[DIFF_EASY] = 0;
    g_highScore[DIFF_NORMAL] = 0;
    g_highScore[DIFF_HARD] = 0;
    if (fopen_s(&f, HIGHSCORE_FILE, "rb") == 0 && f != NULL) {
        fread(g_highScore, sizeof(int), 3, f);
        fclose(f);
    }
}

// 保存最高分到本地文件
static void SaveHighScore() {
    if (g_score <= g_highScore[g_selectedDifficulty]) return;
    g_highScore[g_selectedDifficulty] = g_score;
    FILE* f = NULL;
    if (fopen_s(&f, HIGHSCORE_FILE, "wb") == 0 && f != NULL) {
        fwrite(g_highScore, sizeof(int), 3, f);
        fclose(f);
    }
}

// ====================== 自适应比例贴图绘制函数 ======================
// 【豆包AI辅助】：纹理宽高比计算、自适应缩放算法
static void DrawTextureAutoScale(Texture2D tex, float centerX, float centerY, float targetW, float targetH, Color tint) {
    if (tex.id == 0) {
        // 加载失败显示占位矩形
        DrawRectangle(
            (int)(centerX - targetW / 2 * TEXTURE_GLOBAL_SCALE),
            (int)(centerY - targetH / 2 * TEXTURE_GLOBAL_SCALE),
            (int)(targetW * TEXTURE_GLOBAL_SCALE),
            (int)(targetH * TEXTURE_GLOBAL_SCALE),
            tint
        );
        return;
    }

    // 计算原始宽高比
    float aspectRatio = (float)tex.width / tex.height;
    float targetAspectRatio = targetW / targetH;

    float scale;
    if (aspectRatio > targetAspectRatio) {
        scale = targetW / tex.width * TEXTURE_GLOBAL_SCALE;
    }
    else {
        scale = targetH / tex.height * TEXTURE_GLOBAL_SCALE;
    }

    // 计算居中位置
    float drawX = centerX - tex.width * scale / 2;
    float drawY = centerY - tex.height * scale / 2;

    Vector2 pos;
    pos.x = drawX;
    pos.y = drawY;
    DrawTextureEx(tex, pos, 0, scale, WHITE);
}

// 一次性加载所有贴图 
static void LoadAllTextures() {
    // 加载3辆玩家车
    carTextures[0] = LoadTexture("res/texture/car1.png");
    carTextures[1] = LoadTexture("res/texture/car2.png");
    carTextures[2] = LoadTexture("res/texture/car3.png");

    //  加载障碍物贴图
    obstacleTextures[0] = LoadTexture("res/texture/cat.png");    // 小猫
    obstacleTextures[1] = LoadTexture("res/texture/people.png"); // 行人
    obstacleTextures[2] = LoadTexture("res/texture/bike.png");   // 自行车 
    obstacleTextures[3] = LoadTexture("res/texture/bus.png");    // 公交车 
    obstacleTextures[4] = LoadTexture("res/texture/static.png"); // 路障 

    // 加载道具贴图
    itemTextures[ITEM_LIFE] = LoadTexture("res/texture/life.png");     // 生命 
    itemTextures[ITEM_SCORE] = LoadTexture("res/texture/score.png");   // 分数 
    itemTextures[ITEM_SPEED] = LoadTexture("res/texture/speed.png");   // 加速 
    itemTextures[ITEM_SHIELD] = LoadTexture("res/texture/shield.png"); // 护盾 
    itemTextures[ITEM_SLOW] = LoadTexture("res/texture/slow.png");     // 减速 
    itemTextures[ITEM_NITRO] = LoadTexture("res/texture/nitro.png");   // 氮气 
    itemTextures[ITEM_MAGNET] = LoadTexture("res/texture/magnet.png"); // 磁铁 

    // 加载特殊元素贴图
    ghostTexture = LoadTexture("res/texture/ghost.png");     // 雾天幽灵
    puddleTexture = LoadTexture("res/texture/puddle.png");   // 雨天水坑
    roadblockTexture = LoadTexture("res/texture/roadblock.png"); // 三连路障
    glareTexture = LoadTexture("res/texture/glare.png");     // 太阳眩光

    // 加载启动页+背景贴图
    g_splashTex = LoadTexture("res/texture/background/splash.png");
    g_menuBgTex = LoadTexture("res/texture/background/menu_bg.png");

    // 单张图加载
    g_sunBg[0] = LoadTexture("res/texture/background/sun_bg.png");
    g_fogBg[0] = LoadTexture("res/texture/background/fog_bg.png");
    g_rainBg[0] = LoadTexture("res/texture/background/rain_bg.png");
}

// ====================== 启动页逻辑 ======================
static void UpdateSplash() {
    if (g_splashDone) return;

    g_splashTimer += 1.0f;

    // 阶段1：纯显示3秒
    if (g_splashTimer <= SPLASH_DISPLAY_TIME) {
        g_splashAlpha = 1.0f;
    }
    // 阶段2：1秒淡出【豆包AI辅助】
    else if (g_splashTimer <= SPLASH_DISPLAY_TIME + SPLASH_FADE_TIME) {
        float fadeProgress = (g_splashTimer - SPLASH_DISPLAY_TIME) / SPLASH_FADE_TIME;
        g_splashAlpha = 1.0f - fadeProgress;
    }
    // 阶段3：淡出完成
    else {
        g_splashAlpha = 0.0f;
        g_splashDone = true;
    }
}
//【豆包AI】辅助
static void DrawSplash() {
    if (g_splashDone) return;

    // 自动拉伸铺满全屏，居中显示
    float scaleX = (float)SCREEN_WIDTH / g_splashTex.width;
    float scaleY = (float)SCREEN_HEIGHT / g_splashTex.height;
    float scale = (scaleX > scaleY) ? scaleX : scaleY;
    float drawX = (SCREEN_WIDTH - g_splashTex.width * scale) / 2.0f;
    float drawY = (SCREEN_HEIGHT - g_splashTex.height * scale) / 2.0f;

    DrawTextureEx(g_splashTex, { drawX, drawY }, 0.0f, scale, Fade(WHITE, g_splashAlpha));
}

// ======================【豆包AI辅助】 背景系统 ======================
#define BG_FADE_HEIGHT 100.0f

static void UpdateGameBg() {
    if (g_state != STATE_PLAYING && g_state != STATE_COUNTDOWN) return;

    // 平滑过渡到氮气状态/正常状态
    float targetSpeedMul = g_isNitroActive ? NITRO_BG_SPEED_MUL : 1.0f;
    float targetDarkMul = g_isNitroActive ? NITRO_BG_DARK_MUL : 1.0f;

    g_bgCurrentSpeedMul += (targetSpeedMul - g_bgCurrentSpeedMul) * GetFrameTime() * BG_TRANSITION_SPEED;
    g_bgCurrentDarkMul += (targetDarkMul - g_bgCurrentDarkMul) * GetFrameTime() * BG_TRANSITION_SPEED;

    g_bgOffset += BG_SCROLL_SPEED * g_bgCurrentSpeedMul;

    // 精确重置偏移量
    if (g_bgOffset >= SCREEN_HEIGHT) {
        g_bgOffset -= SCREEN_HEIGHT;
    }
}

static void DrawGameBg() {
    Texture2D* bgSet;
    if (g_currentWeather == WEATHER_RAINY || g_currentWeather == WEATHER_HAIL) {
        bgSet = g_rainBg;
    }
    else if (g_currentWeather == WEATHER_FOGGY) {
        bgSet = g_fogBg;
    }
    else {
        bgSet = g_sunBg;
    }

    // 自动拉伸适配全屏
    float scaleX = (float)SCREEN_WIDTH / bgSet[0].width;
    float scaleY = (float)SCREEN_HEIGHT / bgSet[0].height;
    float scale = (scaleX > scaleY) ? scaleX : scaleY;
    float drawX = (SCREEN_WIDTH - bgSet[0].width * scale) / 2.0f;
    float bgDrawHeight = bgSet[0].height * scale;

    // 应用动态亮度
    float baseAlpha = BG_DARK_ALPHA * g_bgCurrentDarkMul;

    // 绘制基础的两张图
    DrawTextureEx(bgSet[0], { drawX, g_bgOffset }, 0.0f, scale, Fade(WHITE, baseAlpha));
    DrawTextureEx(bgSet[0], { drawX, g_bgOffset - bgDrawHeight }, 0.0f, scale, Fade(WHITE, baseAlpha));

    // 边缘羽化过渡（只有接缝进入屏幕时才生效）
    if (g_bgOffset > SCREEN_HEIGHT - BG_FADE_HEIGHT) {
        float transition = (g_bgOffset - (SCREEN_HEIGHT - BG_FADE_HEIGHT)) / BG_FADE_HEIGHT;

        // 第一张图底部淡出
        Color bottomFade = Fade(WHITE, baseAlpha * (1.0f - transition));
        DrawTextureEx(bgSet[0], { drawX, g_bgOffset }, 0.0f, scale, bottomFade);

        // 第二张图顶部淡入
        Color topFade = Fade(WHITE, baseAlpha * transition);
        DrawTextureEx(bgSet[0], { drawX, g_bgOffset - bgDrawHeight }, 0.0f, scale, topFade);
    }
}
// ======================速度线系统======================
// 初始化氮气速度线
static void InitSpeedLines() {
    for (int i = 0; i < SPEED_LINE_MAX; i++) {
        g_speedLines[i].x = (float)GetRandomValue(ROAD_X, ROAD_X + ROAD_WIDTH);
        g_speedLines[i].y = (float)GetRandomValue(-SCREEN_HEIGHT, 0);
        g_speedLines[i].spd = (float)GetRandomValue(20, 35);
    }
}

// 更新速度线位置
static void UpdateSpeedLines() {
    if (!g_isNitroActive) return;
    for (int i = 0; i < SPEED_LINE_MAX; i++) {
        g_speedLines[i].y += g_speedLines[i].spd * 2.0f;
        if (g_speedLines[i].y > SCREEN_HEIGHT) {
            g_speedLines[i].x = (float)GetRandomValue(ROAD_X, ROAD_X + ROAD_WIDTH);
            g_speedLines[i].y = (float)GetRandomValue(-50, 0);
            g_speedLines[i].spd = (float)GetRandomValue(20, 35);
        }
    }
}

// 绘制速度线
static void DrawSpeedLines() {
    if (!g_isNitroActive) return;
    for (int i = 0; i < SPEED_LINE_MAX; i++) {
        DrawRectangle(
            (int)(g_speedLines[i].x + g_shakeOffsetX),
            (int)(g_speedLines[i].y + g_shakeOffsetY),
            2,
            40,
            Fade(WHITE, 0.6f)
        );
    }
}

// ====================== 雨水坑系统======================
static void InitPuddle() {
    for (int i = 0; i < PUDDLE_MAX; i++) {
        g_puddleList[i].active = false;
    }
}

static void SpawnPuddle() {
    for (int i = 0; i < PUDDLE_MAX; i++) {
        if (!g_puddleList[i].active) {
            g_puddleList[i].w = (float)GetRandomValue(48, 68);
            g_puddleList[i].h = (float)GetRandomValue(32, 42);

            // 【豆包AI辅助】：最多重试5次找不重叠的位置
            int retry = 0;
            do {
                g_puddleList[i].x = (float)GetRandomValue((int)(ROAD_X + 20), (int)(ROAD_X + ROAD_WIDTH - 20 - (int)g_puddleList[i].w));
                g_puddleList[i].y = -g_puddleList[i].h;
                retry++;
            } while (IsAreaOccupied(g_puddleList[i].x + g_puddleList[i].w / 2, g_puddleList[i].y, g_puddleList[i].w, g_puddleList[i].h) && retry < 5);

            if (retry >= 5) return;

            g_puddleList[i].active = true;
            g_puddleList[i].hasItem = (GetRandomValue(0, 3) == 0);
            break;
        }
    }
}

// 更新水坑状态
static void UpdatePuddle() {
    if (g_currentWeather != WEATHER_RAINY || g_isWeatherChanging) return;
    static int spawnTimer = 0;
    spawnTimer++;
    if (spawnTimer >= 300) {
        SpawnPuddle();
        spawnTimer = 0;
    }

    for (int i = 0; i < PUDDLE_MAX; i++) {
        if (g_puddleList[i].active) {
            g_puddleList[i].y += g_obsBaseSpd;
            if (g_puddleList[i].y > SCREEN_HEIGHT) g_puddleList[i].active = false;
        }
    }
}

// ====================== 太阳眩光系统 ======================
// 更新眩光状态
static void UpdateSunGlare() {
    if (g_currentWeather != WEATHER_SUNNY || g_isWeatherChanging || g_state != STATE_PLAYING || g_isPause) return;

    g_glareIntervalTimer++;
    if (!g_sunGlare.active && g_glareIntervalTimer >= GetRandomValue((int)GLARE_INTERVAL_MIN, (int)GLARE_INTERVAL_MAX)) {
        g_sunGlare.active = true;
        g_sunGlare.timer = GLARE_DURATION;
        g_glareIntervalTimer = 0;
        ShowPopup(tipGlare[GetRandomValue(0, 2)], YELLOW);
    }

    if (g_sunGlare.active) {
        g_sunGlare.timer--;
        if (g_sunGlare.timer <= 0) {
            g_sunGlare.active = false;
        }
    }
}

//绘制太阳眩光
static void DrawSunGlare() {
    if (!g_sunGlare.active) return;
    float alpha = 0.6f;
    if (g_sunGlare.timer < 20) alpha = g_sunGlare.timer / 20.0f * 0.6f;
    if (g_sunGlare.timer > GLARE_DURATION - 20) alpha = (GLARE_DURATION - g_sunGlare.timer) / 20.0f * 0.6f;

    if (glareTexture.id != 0) {
        float scale = (float)ROAD_WIDTH / glareTexture.width * 0.9f;
        float drawX = ROAD_X + (ROAD_WIDTH - glareTexture.width * scale) / 2 + g_shakeOffsetX;
        float drawY = 200 + g_shakeOffsetY;

        Vector2 pos;
        pos.x = drawX;
        pos.y = drawY;
        DrawTextureEx(glareTexture, pos, 0, scale, Fade(WHITE, alpha));
    }
    else {
        // 加载失败回退到原来的矩形
        DrawRectangle(ROAD_X + 100, 200, ROAD_WIDTH - 200, 300, Fade(WHITE, alpha));
    }
}

// ====================== 幽灵障碍物系统 ======================
// 初始化幽灵障碍物
static void InitGhostObstacle() {
    for (int i = 0; i < GHOST_OBSTACLE_MAX; i++) {
        g_ghostObsList[i].active = false;
    }
}

//生成幽灵障碍物
static void SpawnGhostObstacle() {
    for (int i = 0; i < GHOST_OBSTACLE_MAX; i++) {
        if (!g_ghostObsList[i].active) {
            int r = GetRandomValue(0, 4);
            g_ghostObsList[i].type = (ObstacleType)r;
            // 设置幽灵障碍物尺寸
            if (r == 0) { g_ghostObsList[i].w = 42; g_ghostObsList[i].h = 52; } // 小猫
            else if (r == 1) { g_ghostObsList[i].w = 52; g_ghostObsList[i].h = 105; } // 行人
            else if (r == 2) { g_ghostObsList[i].w = 52; g_ghostObsList[i].h = 72; } // 自行车
            else if (r == 3) { g_ghostObsList[i].w = 82; g_ghostObsList[i].h = 92; } // 公交车
            else { g_ghostObsList[i].w = 62; g_ghostObsList[i].h = 105; } // 静态路障

            int retry = 0;
            do {
                g_ghostObsList[i].x = (float)GetRandomValue((int)(ROAD_X + 20), (int)(ROAD_X + ROAD_WIDTH - 20));
                g_ghostObsList[i].y = -g_ghostObsList[i].h;
                retry++;
            } while (IsAreaOccupied(g_ghostObsList[i].x, g_ghostObsList[i].y, g_ghostObsList[i].w, g_ghostObsList[i].h) && retry < 5);

            if (retry >= 5) return;

            g_ghostObsList[i].spdY = g_obsBaseSpd * 0.9f;
            g_ghostObsList[i].active = true;
            break;
        }
    }
}

// 更新幽灵障碍物
static void UpdateGhostObstacle() {
    if (g_currentWeather != WEATHER_FOGGY || g_isWeatherChanging) return;
    g_ghostSpawnTimer++;
    if (g_ghostSpawnTimer >= GHOST_SPAWN_INTERVAL) {
        SpawnGhostObstacle();
        g_ghostSpawnTimer = 0;
    }

    for (int i = 0; i < GHOST_OBSTACLE_MAX; i++) {
        if (g_ghostObsList[i].active) {
            g_ghostObsList[i].y += g_ghostObsList[i].spdY;
            if (g_ghostObsList[i].y > SCREEN_HEIGHT) g_ghostObsList[i].active = false;
        }
    }
}

// ====================== 冰雹风暴系统 ======================
// 初始化冰雹
static void InitHail() {
    for (int i = 0; i < HAIL_MAX; i++) {
        g_hailList[i].active = false;
    }
}

//生成冰雹
static void SpawnHail() {
    for (int i = 0; i < HAIL_MAX; i++) {
        if (!g_hailList[i].active) {
            g_hailList[i].w = 15;
            g_hailList[i].h = 15;
            g_hailList[i].x = (float)GetRandomValue(ROAD_X, ROAD_X + ROAD_WIDTH);
            g_hailList[i].y = (float)GetRandomValue(-SCREEN_HEIGHT, 0);
            g_hailList[i].spd = (float)GetRandomValue(20, 30);
            g_hailList[i].active = true;
            break;
        }
    }
}

//更新冰雹状态
static void UpdateHail() {
    if (g_currentWeather != WEATHER_HAIL || g_isWeatherChanging) return;
    static int spawnTimer = 0;
    spawnTimer++;
    if (spawnTimer >= 20) {
        SpawnHail();
        spawnTimer = 0;
    }

    for (int i = 0; i < HAIL_MAX; i++) {
        if (g_hailList[i].active) {
            g_hailList[i].y += g_hailList[i].spd;
            if (g_hailList[i].y > SCREEN_HEIGHT) g_hailList[i].active = false;
        }
    }
}

//绘制冰雹
static void DrawHail() {
    if (g_hailAlpha <= 0.01f) return;
    for (int i = 0; i < HAIL_MAX; i++) {
        if (g_hailList[i].active) {
            DrawRectangle(
                (int)(g_hailList[i].x + g_shakeOffsetX),
                (int)(g_hailList[i].y + g_shakeOffsetY),
                15,
                15,
                Fade(SKYBLUE, 0.9f * g_hailAlpha)
            );
        }
    }
}

// ====================== 路障系统 ======================
// 初始化路障
static void InitRoadBlock() {
    for (int i = 0; i < ROADBLOCK_MAX; i++) {
        g_roadBlockList[i].active = false;
    }
}

// 生成三连路障
static void SpawnRoadBlock() {
    int blockLane = GetRandomValue(0, LANE_COUNT - 1);
    for (int i = 0; i < ROADBLOCK_MAX; i++) {
        if (!g_roadBlockList[i].active) {
            g_roadBlockList[i].lane = blockLane;
            g_roadBlockList[i].w = 72;
            g_roadBlockList[i].h = 82;
            g_roadBlockList[i].x = g_laneCenter[blockLane] - g_roadBlockList[i].w / 2;
            g_roadBlockList[i].y = -g_roadBlockList[i].h - i * 200;
            g_roadBlockList[i].active = true;
        }
    }
    ShowPopup(tipRoadBlock[GetRandomValue(0, 2)], ORANGE);
}

// 更新路障状态
static void UpdateRoadBlock() {
    if (g_state != STATE_PLAYING || g_isPause) return;
    g_roadBlockIntervalTimer++;
    if (g_roadBlockIntervalTimer >= ROADBLOCK_INTERVAL) {
        SpawnRoadBlock();
        g_roadBlockIntervalTimer = 0;
    }

    for (int i = 0; i < ROADBLOCK_MAX; i++) {
        if (g_roadBlockList[i].active) {
            g_roadBlockList[i].y += g_obsBaseSpd;
            if (g_roadBlockList[i].y > SCREEN_HEIGHT) g_roadBlockList[i].active = false;
        }
    }
}

// ====================== 加速带系统 ======================
// 初始化加速带
static void InitSpeedBelt() {
    for (int i = 0; i < SPEEDBELT_MAX; i++) {
        g_speedBeltList[i].active = false;
    }
}

//生成加速带
static void SpawnSpeedBelt() {
    for (int i = 0; i < SPEEDBELT_MAX; i++) {
        if (!g_speedBeltList[i].active) {
            g_speedBeltList[i].w = (float)ROAD_WIDTH;
            g_speedBeltList[i].h = 80;
            g_speedBeltList[i].x = (float)ROAD_X;
            g_speedBeltList[i].y = -g_speedBeltList[i].h;
            g_speedBeltList[i].active = true;
            break;
        }
    }
}

// 更新加速带状态
static void UpdateSpeedBelt() {
    if (g_state != STATE_PLAYING || g_isPause) return;
    g_speedBeltIntervalTimer++;
    if (g_speedBeltIntervalTimer >= SPEEDBELT_INTERVAL) {
        SpawnSpeedBelt();
        g_speedBeltIntervalTimer = 0;
    }

    for (int i = 0; i < SPEEDBELT_MAX; i++) {
        if (g_speedBeltList[i].active) {
            g_speedBeltList[i].y += g_obsBaseSpd;
            if (g_speedBeltList[i].y > SCREEN_HEIGHT) g_speedBeltList[i].active = false;
        }
    }

    if (g_beltSpeedBuffTimer > 0) g_beltSpeedBuffTimer--;
}

// ====================== 天气核心系统 ======================
// 初始化雨滴
static void InitRain() {
    for (int i = 0; i < RAIN_MAX; i++) {
        g_rainList[i].x = (float)GetRandomValue(ROAD_X, ROAD_X + ROAD_WIDTH);
        g_rainList[i].y = (float)GetRandomValue(-SCREEN_HEIGHT, 0);
        g_rainList[i].spd = (float)GetRandomValue(15, 25);
    }
}

// 随机切换天气
static void RandomWeather(bool isInit) {
    int newWeather;
    do {
        newWeather = GetRandomValue(0, 3);
    } while (newWeather == g_currentWeather && !isInit);

    if (isInit) {
        g_currentWeather = newWeather;
        g_weatherTimer = (float)GetRandomValue((int)WEATHER_MIN_INTERVAL, (int)WEATHER_MAX_INTERVAL);
        g_isWeatherChanging = false;
        g_weatherTransition = 0.0f;

        InitPuddle();
        InitGhostObstacle();
        InitHail();

        if (g_currentWeather == WEATHER_RAINY) {
            InitRain();
            g_rainAlpha = 1.0f;
            g_hailAlpha = 0.0f;
        }
        else if (g_currentWeather == WEATHER_HAIL) {
            InitHail();
            g_hailAlpha = 1.0f;
            g_rainAlpha = 0.0f;
        }
        else {
            g_rainAlpha = 0.0f;
            g_hailAlpha = 0.0f;
        }
        if (g_currentWeather == WEATHER_FOGGY) {
            g_fogAlpha = 0.6f;
        }
        else {
            g_fogAlpha = 0.0f;
        }
    }
    else {
        g_nextWeather = newWeather;
        g_isWeatherChanging = true;
        g_weatherTransition = 0.0f;
    }
}

// 更新天气系统
// 【豆包AI辅助】天气平滑过渡
static void UpdateWeather() {
    if (g_state != STATE_PLAYING || g_isPause) return;

    if (g_isWeatherChanging) {
        g_weatherTransition += 1.0f;
        float t = g_weatherTransition / WEATHER_TRANSITION_TIME;
        if (t > 1.0f) t = 1.0f;

        if (t >= 1.0f) {
            g_currentWeather = g_nextWeather;
            g_isWeatherChanging = false;
            g_weatherTimer = (float)GetRandomValue((int)WEATHER_MIN_INTERVAL, (int)WEATHER_MAX_INTERVAL);

            // 切换天气后显示提示
            if (g_currentWeather == WEATHER_SUNNY) {
                ShowPopup(tipWeatherSunny[GetRandomValue(0, 2)], YELLOW);
            }
            else if (g_currentWeather == WEATHER_RAINY) {
                InitRain();
                InitPuddle();
                ShowPopup(tipWeatherRainy[GetRandomValue(0, 2)], SKYBLUE);
            }
            else if (g_currentWeather == WEATHER_FOGGY) {
                InitGhostObstacle();
                ShowPopup(tipWeatherFoggy[GetRandomValue(0, 2)], LIGHTGRAY);
            }
            else if (g_currentWeather == WEATHER_HAIL) {
                InitHail();
                ShowPopup(tipWeatherHail[GetRandomValue(0, 2)], SKYBLUE);
            }
        }
        else {
            // 线性插值计算天气透明度，实现平滑过渡
            int fromWeather = g_currentWeather;
            int toWeather = g_nextWeather;

            float fromRain = (fromWeather == WEATHER_RAINY) ? 1.0f : 0.0f;
            float toRain = (toWeather == WEATHER_RAINY) ? 1.0f : 0.0f;
            g_rainAlpha = fromRain * (1 - t) + toRain * t;

            float fromHail = (fromWeather == WEATHER_HAIL) ? 1.0f : 0.0f;
            float toHail = (toWeather == WEATHER_HAIL) ? 1.0f : 0.0f;
            g_hailAlpha = fromHail * (1 - t) + toHail * t;

            float fromFog = (fromWeather == WEATHER_FOGGY) ? 0.6f : 0.0f;
            float toFog = (toWeather == WEATHER_FOGGY) ? 0.6f : 0.0f;
            g_fogAlpha = fromFog * (1 - t) + toFog * t;

            // 过渡到50%时预加载天气效果
            if (toWeather == WEATHER_RAINY && t > 0.5f && fromWeather != WEATHER_RAINY) InitRain();
            if (toWeather == WEATHER_HAIL && t > 0.5f && fromWeather != WEATHER_HAIL) InitHail();
        }
        return;
    }

    g_weatherTimer -= 1.0f;
    if (g_weatherTimer <= 0) {
        RandomWeather(false);
    }

    // 雾天动态透明度效果
    if (g_currentWeather == WEATHER_FOGGY && !g_isWeatherChanging) {
        g_fogAlpha = 0.6f + (float)sin(GetTime() * 2) * 0.08f;
    }
}

// 更新雨滴位置
static void UpdateRain() {
    if (g_rainAlpha <= 0.01f) return;
    for (int i = 0; i < RAIN_MAX; i++) {
        g_rainList[i].y += g_rainList[i].spd;
        if (g_rainList[i].y > SCREEN_HEIGHT) {
            g_rainList[i].x = (float)GetRandomValue(ROAD_X, ROAD_X + ROAD_WIDTH);
            g_rainList[i].y = (float)GetRandomValue(-50, 0);
            g_rainList[i].spd = (float)GetRandomValue(15, 25);
        }
    }
}

// 绘制雨滴
static void DrawRain() {
    if (g_rainAlpha <= 0.01f) return;
    for (int i = 0; i < RAIN_MAX; i++) {
        Vector2 start, end;
        start.x = g_rainList[i].x + g_shakeOffsetX;
        start.y = g_rainList[i].y + g_shakeOffsetY;
        end.x = g_rainList[i].x + g_shakeOffsetX + 2.0f;
        end.y = g_rainList[i].y + g_shakeOffsetY + 12.0f;
        DrawLineEx(start, end, 1.5f, Fade(SKYBLUE, 0.7f * g_rainAlpha));
    }
}

// 绘制雾效
static void DrawFog() {
    if (g_fogAlpha <= 0.01f) return;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        float alpha = g_fogAlpha;
        if (y < 200) alpha *= (float)y / 200.0f;
        if (y > SCREEN_HEIGHT - 300) alpha *= (float)(SCREEN_HEIGHT - y) / 300.0f;
        DrawRectangle(0, y, SCREEN_WIDTH, 1, Fade(BLACK, alpha));
    }
}

// ====================== 难度设置 ======================
// 应用选择的游戏难度
static void ApplyDifficulty() {
    if (g_selectedDifficulty == DIFF_EASY) {
        g_obsBaseSpd = 3.0f;
        g_spawnGap = 140;
    }
    if (g_selectedDifficulty == DIFF_NORMAL) {
        g_obsBaseSpd = 5.0f;
        g_spawnGap = 100;
    }
    if (g_selectedDifficulty == DIFF_HARD) {
        g_obsBaseSpd = 7.0f;
        g_spawnGap = 70;
    }
}

// ====================== 尾气粒子系统 ======================
// 生成尾气粒子
// 【豆包AI辅助】：粒子系统生命周期管理
static void SpawnExhaust() {
    int spawnCount = g_isNitroActive ? 3 : 1;
    for (int s = 0; s < spawnCount; s++) {
        for (int i = 0; i < EXHAUST_MAX; i++) {
            if (g_exhaust[i].life <= 0) {
                g_exhaust[i].x = g_player.x + (float)GetRandomValue(-12, 12);
                g_exhaust[i].y = g_player.y + 90.0f + (float)GetRandomValue(0, 10);
                g_exhaust[i].life = g_isNitroActive ? 40.0f : 30.0f;
               
                // 根据玩家状态设置尾气颜色
                if (g_isNitroActive) {
                    g_exhaust[i].col = (GetRandomValue(0, 1) == 0) ? ORANGE : RED;
                }
                else if (g_shieldTimer > 0)      g_exhaust[i].col = PURPLE;
                else if (g_speedBuffTimer > 0) g_exhaust[i].col = SKYBLUE;
                else if (g_slowTimer > 0)    g_exhaust[i].col = DARKGRAY;
                else                        g_exhaust[i].col = LIGHTGRAY;
                break;
            }
        }
    }
}

// 更新尾气粒子
static void UpdateExhaust() {
    for (int i = 0; i < EXHAUST_MAX; i++) {
        if (g_exhaust[i].life > 0) {
            g_exhaust[i].y += g_isNitroActive ? 3.0f : 1.5f;
            g_exhaust[i].life -= 1.0f;
            g_exhaust[i].col.a = (unsigned char)(g_exhaust[i].life / 40.0f * 255);
        }
    }
}

// ====================== 屏幕抖动系统 ======================
// 更新屏幕抖动效果
static void UpdateShake() {
    if (g_isNitroActive) {
        g_shakeOffsetX = (float)GetRandomValue(-3, 3);
        g_shakeOffsetY = (float)GetRandomValue(-2, 2);
        return;
    }

    if (g_shakeTimer > 0) {
        g_shakeOffsetX = (float)GetRandomValue(-6, 6);
        g_shakeOffsetY = (float)GetRandomValue(-6, 6);
        g_shakeTimer -= 1.0f;
    }
    else {
        g_shakeOffsetX = 0;
        g_shakeOffsetY = 0;
    }
}

// ====================== 初始化函数 ======================
// 初始化车道位置
static void InitLanePos() {
    g_laneWidth = (float)ROAD_WIDTH / LANE_COUNT;
    g_laneCenter[0] = ROAD_X + g_laneWidth * 0.5f;
    g_laneCenter[1] = ROAD_X + g_laneWidth * 1.5f;
    g_laneCenter[2] = ROAD_X + g_laneWidth * 2.5f;
}

// 初始化玩家属性
static void InitPlayer() {
    g_player.x = ROAD_X + ROAD_WIDTH / 2.0f;
    g_player.y = 680.0f;
    g_player.hp = MAX_HP;
    g_player.carId = g_selectedCar;
    g_speedBuffTimer = g_shieldTimer = g_slowTimer = 0;
    g_nitroValue = 0.0f;
    g_isNitroActive = false;
    g_nitroBurstTimer = 0.0f;
    g_playerMoveVelX = 0.0f;
    g_comboCount = 0;
    g_comboTimer = 0.0f;
    g_slipTimer = 0.0f;
    g_beltSpeedBuffTimer = 0.0f;
    g_glareIntervalTimer = 0.0f;
    g_sunGlare.active = false;
    g_roadBlockIntervalTimer = 0.0f;
    g_speedBeltIntervalTimer = 0.0f;
    g_magnetTimer = 0.0f;

    // 重置背景状态
    g_bgOffset = 0.0f;
    g_bgCurrentSpeedMul = 1.0f;
    g_bgCurrentDarkMul = 1.0f;

    // 初始化所有场景元素
    InitPuddle();
    InitGhostObstacle();
    InitHail();
    InitRoadBlock();
    InitSpeedBelt();
    InitSpeedLines();
}

// 初始化障碍物数组
static void InitObstacles() { for (int i = 0; i < MAX_OBSTACLE; i++) g_obsList[i].active = false; }
// 初始化道具数组
static void InitItems() { for (int i = 0; i < MAX_ITEM; i++) g_itemList[i].active = false; }

// ====================== 障碍物生成 ======================
// 生成动态障碍物（猫、人、自行车、公交车）
static void SpawnDynamicObs() {
    for (int i = 0; i < MAX_OBSTACLE; i++) {
        if (!g_obsList[i].active) {
            int r = GetRandomValue(0, 3);
            g_obsList[i].type = (ObstacleType)r;
            // 动态障碍物尺寸
            if (r == 0) { g_obsList[i].w = 48; g_obsList[i].h = 60; } // 小猫
            else if (r == 1) { g_obsList[i].w = 100; g_obsList[i].h = 200; } // 行人
            else if (r == 2) { g_obsList[i].w = 60; g_obsList[i].h = 85; } // 自行车
            else { g_obsList[i].w = 95; g_obsList[i].h = 105; } // 公交车

            int retry = 0;
            do {
                g_obsList[i].x = (float)GetRandomValue((int)(ROAD_X + 20), (int)(ROAD_X + ROAD_WIDTH - 20));
                g_obsList[i].y = -g_obsList[i].h;
                retry++;
            } while (IsAreaOccupied(g_obsList[i].x, g_obsList[i].y, g_obsList[i].w, g_obsList[i].h) && retry < 5);

            if (retry >= 5) return;

            //设置障碍物移动速度
            float add = g_obsBaseSpd - 5.0f;
            float speedMul = g_isNitroActive ? 1.2f : 1.0f;
            if (r == 2) g_obsList[i].spdY = (SPD_BIKE + add) * speedMul;
            else if (r == 0) g_obsList[i].spdY = (SPD_CAT + add) * speedMul;
            else if (r == 3) g_obsList[i].spdY = (SPD_BUS + add) * speedMul;
            else g_obsList[i].spdY = (SPD_PEOPLE + add) * speedMul * 0.8f;
            g_obsList[i].spdX = (float)GetRandomValue(-15, 15) / 10.0f;
            g_obsList[i].active = true;
            break;
        }
    }
}

// 生成静态路障
static void SpawnStaticObs() {
    for (int i = 0; i < MAX_OBSTACLE; i++) {
        if (!g_obsList[i].active) {
            g_obsList[i].type = OBJ_STATIC;
            g_obsList[i].w = 62; g_obsList[i].h = 105;
            int laneW = (int)g_laneWidth;

            int retry = 0;
            do {
                g_obsList[i].x = (GetRandomValue(0, 1) == 0) ?
                    (float)GetRandomValue((int)(ROAD_X + 20), (int)(ROAD_X + laneW - 20)) :
                    (float)GetRandomValue((int)(ROAD_X + laneW * 2 + 20), (int)(ROAD_X + ROAD_WIDTH - 20));
                g_obsList[i].y = -105.0f;
                retry++;
            } while (IsAreaOccupied(g_obsList[i].x, g_obsList[i].y, g_obsList[i].w, g_obsList[i].h) && retry < 5);

            if (retry >= 5) return;

            float speedMul = g_isNitroActive ? 1.2f : 1.0f;
            g_obsList[i].spdY = SPD_STATIC * speedMul;
            g_obsList[i].spdX = 0;
            g_obsList[i].active = true;
            break;
        }
    }
}

// ====================== 道具生成 ======================
// 生成随机道具
static void SpawnItem() {
    for (int i = 0; i < MAX_ITEM; i++) {
        if (!g_itemList[i].active) {
            // // 随机分配道具类型，磁铁生成概率20%，氮气40%，其他40%
            int rnd = GetRandomValue(0, 4);
            if (rnd == 0)
                g_itemList[i].type = (ItemType)ITEM_MAGNET;
            else if (rnd <= 2)
                g_itemList[i].type = (ItemType)ITEM_NITRO;
            else
                g_itemList[i].type = (ItemType)GetRandomValue(0, 4);

            g_itemList[i].w = 36;
            g_itemList[i].h = 36;

            int retry = 0;
            do {
                g_itemList[i].x = (float)GetRandomValue((int)(ROAD_X + 20), (int)(ROAD_X + ROAD_WIDTH - 20));
                g_itemList[i].y = -50.0f;
                retry++;
            } while (IsAreaOccupied(g_itemList[i].x, g_itemList[i].y, 36, 36) && retry < 5);

            if (retry >= 5) return;

            float speedMul = g_isNitroActive ? 1.2f : 1.0f;
            g_itemList[i].spdY = g_obsBaseSpd * 0.8f * speedMul;
            g_itemList[i].active = true;
            break;
        }
    }
}

// 水坑生成分数道具
static void SpawnPuddleItem(float x, float y) {
    for (int i = 0; i < MAX_ITEM; i++) {
        if (!g_itemList[i].active) {
            g_itemList[i].type = (ItemType)ITEM_SCORE;
            g_itemList[i].w = 36;
            g_itemList[i].h = 36;
            g_itemList[i].x = x;
            g_itemList[i].y = y;
            g_itemList[i].spdY = g_obsBaseSpd;
            g_itemList[i].active = true;
            break;
        }
    }
}

// ====================== 游戏初始化 ======================
void GameInit() {
    static bool firstLoad = true;
    if (firstLoad) {
        LoadAllTextures(); // 游戏启动时一次性加载所有贴图
        LoadHighScore();
        firstLoad = false;
    }
    InitLanePos();
    g_state = (GameState)STATE_DIFFICULTY;
    g_score = g_flashTimer = g_shakeTimer = 0;
    g_popup.timer = 0;
    g_spawnTimer = g_staticSpawnTimer = 0;
    g_isPause = false;
    RandomWeather(true);
    InitPlayer();
    InitObstacles();
    InitItems();
}

// ====================== 玩家更新 ======================
static void UpdatePlayer() {
    float speedMul = 1.0f;
    if (g_isNitroActive) {
        if (g_nitroBurstTimer > 0) {
            speedMul = NITRO_BURST_MUL;
        }
        else {
            speedMul = NITRO_SUSTAIN_MUL;
        }
    }
    else if (g_slowTimer > 0) {
        speedMul = 0.4f;
    }
    // 加速带效果
    if (g_beltSpeedBuffTimer > 0) {
        speedMul *= 2.0f;
    }
    // 计算玩家基础速度
    float diff = g_obsBaseSpd - 5.0f;
    float baseSpeed = (PLAYER_BASE_SPEED + diff * 0.25f) * speedMul;
    float speedMax = g_isNitroActive ? PLAYER_SPEED_MAX * 4 : PLAYER_SPEED_MAX * 2;
    if (baseSpeed > speedMax) baseSpeed = speedMax;
    if (g_speedBuffTimer > 0 && !g_isNitroActive) baseSpeed *= 1.5f;

    // 雨天打滑物理效果
    float controlMul = g_isNitroActive ? 1.5f : 1.0f;
    bool isRainy = (g_currentWeather == WEATHER_RAINY && !g_isWeatherChanging) || (g_isWeatherChanging && g_rainAlpha > 0.5f);
    float rainFriction = g_isNitroActive ? 0.95f : 0.85f;
    float rainAccelMul = g_isNitroActive ? 1.0f : 0.3f;

    if (g_slipTimer > 0) {
        g_slipTimer--;
        g_player.x += (float)GetRandomValue(-8, 8);
        g_playerMoveVelX = 0;
    }
    else if (isRainy) {
        float accel = baseSpeed * rainAccelMul * controlMul;
        float friction = rainFriction;
        if (IsKeyDown(KEY_LEFT))  g_playerMoveVelX -= accel;
        if (IsKeyDown(KEY_RIGHT)) g_playerMoveVelX += accel;
        g_playerMoveVelX *= friction;
        g_player.x += g_playerMoveVelX;
    }
    else {
        g_playerMoveVelX = 0.0f;
        float moveSpeed = baseSpeed * controlMul;
        if (IsKeyDown(KEY_LEFT))  g_player.x -= moveSpeed;
        if (IsKeyDown(KEY_RIGHT)) g_player.x += moveSpeed;
    }

    // 玩家垂直移动
    float verticalSpeed = g_isNitroActive ? 8.0f : 5.0f;
    if (IsKeyDown(KEY_W)) g_player.y -= verticalSpeed;
    if (IsKeyDown(KEY_S)) g_player.y += verticalSpeed;

    // 玩家边界限制，不跑出屏幕
    if (g_player.y < 350) g_player.y = 350;
    if (g_player.y > 820) g_player.y = 820;
    if (g_player.x < ROAD_X + 40) {
        g_player.x = ROAD_X + 40;
        g_playerMoveVelX = 0;
    }
    if (g_player.x > ROAD_X + ROAD_WIDTH - 40) {
        g_player.x = ROAD_X + ROAD_WIDTH - 40;
        g_playerMoveVelX = 0;
    }

    // 随机生成尾气
    if (GetRandomValue(0, 2) == 0) SpawnExhaust();
}

// ====================== 氮气系统======================
static void UpdateNitro() {
    bool prevNitroState = g_isNitroActive;
    // 空格触发氮气
    if (IsKeyDown(KEY_SPACE) && g_nitroValue > 0 && g_state == STATE_PLAYING && !g_isPause) {
        g_isNitroActive = true;
        g_nitroValue -= NITRO_CONSUME_RATE;
        if (g_nitroValue < 0) g_nitroValue = 0;

        if (g_nitroBurstTimer > 0) {
            g_nitroBurstTimer -= 1.0f;
        }
    }
    else {
        g_isNitroActive = false;
        g_nitroBurstTimer = 0.0f;
    }

    // 刚开启氮气时触发特效和提示
    if (!prevNitroState && g_isNitroActive) {
        g_flashTimer = 8.0f;
        g_nitroBurstTimer = NITRO_BURST_TIME;
        ShowPopup(tipNitro[GetRandomValue(0, 2)], ORANGE);
    }
}

// ====================== 障碍物/道具 ======================
static void UpdateObs() {
    //更新障碍物位置
    for (int i = 0; i < MAX_OBSTACLE; i++) {
        if (g_obsList[i].active) {
            g_obsList[i].y += g_obsList[i].spdY;
            if (g_obsList[i].type != OBJ_STATIC) g_obsList[i].x += g_obsList[i].spdX;
            if (g_obsList[i].y > SCREEN_HEIGHT) g_obsList[i].active = false;

            // 障碍物边界限制
            if (g_obsList[i].x < ROAD_X + 20) {
                g_obsList[i].x = ROAD_X + 20;
                g_obsList[i].spdX = -g_obsList[i].spdX;
            }
            if (g_obsList[i].x > ROAD_X + ROAD_WIDTH - 20) {
                g_obsList[i].x = ROAD_X + ROAD_WIDTH - 20;
                g_obsList[i].spdX = -g_obsList[i].spdX;
            }
        }
    }

    // 障碍物之间碰撞检测
    for (int i = 0; i < MAX_OBSTACLE; i++) {
        if (!g_obsList[i].active) continue;
        for (int j = i + 1; j < MAX_OBSTACLE; j++) {
            if (!g_obsList[j].active) continue;
            ResolveCollision(&g_obsList[i], &g_obsList[j]);
        }
    }
}

static void UpdateItems() {
    // 磁铁自动吸引逻辑
    // 【豆包AI辅助】：向量距离计算、磁吸
    if (g_magnetTimer > 0) {
        g_magnetTimer--;
        for (int i = 0; i < MAX_ITEM; i++) {
            if (g_itemList[i].active) {
                // 磁铁不吸减速道具
                if (g_itemList[i].type == ITEM_SLOW) {
                    continue;
                }
                // 计算玩家与道具的距离和方向
                float dx = g_player.x - g_itemList[i].x;
                float dy = g_player.y - g_itemList[i].y;
                float dist = sqrtf(dx * dx + dy * dy);
                // 350像素范围内自动吸附
                if (dist < 350.0f) { 
                    g_itemList[i].x += dx * 0.08f;
                    g_itemList[i].y += dy * 0.08f;
                }
            }
        }
    }

    // 更新道具位置
    for (int i = 0; i < MAX_ITEM; i++) {
        if (g_itemList[i].active) {
            g_itemList[i].y += g_itemList[i].spdY;
            if (g_itemList[i].y > SCREEN_HEIGHT) g_itemList[i].active = false;
        }
    }

    // 道具和障碍物碰撞偏移
    for (int i = 0; i < MAX_ITEM; i++) {
        if (!g_itemList[i].active) continue;
        for (int j = 0; j < MAX_OBSTACLE; j++) {
            if (!g_obsList[j].active) continue;

            float iLeft = g_itemList[i].x - 18;
            float iRight = g_itemList[i].x + 18;
            float iTop = g_itemList[i].y;
            float iBottom = g_itemList[i].y + 36;

            float oLeft = g_obsList[j].x - g_obsList[j].w / 2;
            float oRight = g_obsList[j].x + g_obsList[j].w / 2;
            float oTop = g_obsList[j].y;
            float oBottom = g_obsList[j].y + g_obsList[j].h;

            bool hit = !(iRight < oLeft || iLeft > oRight || iBottom < oTop || iTop > oBottom);
            if (hit) {
                // 道具碰到障碍物，稍微偏移一点继续下落
                g_itemList[i].x += (g_itemList[i].x > g_obsList[j].x) ? 10 : -10;
            }
        }
    }
}

// ====================== 碰撞检测 ======================
static void CheckCollision() {
    float pLeft = g_player.x - 52.5f, pRight = g_player.x + 52.5f;
    float pTop = g_player.y, pBottom = g_player.y + 130;

    // 水坑碰撞
    for (int i = 0; i < PUDDLE_MAX; i++) {
        if (g_puddleList[i].active) {
            float oLeft = g_puddleList[i].x, oRight = g_puddleList[i].x + g_puddleList[i].w;
            float oTop = g_puddleList[i].y, oBottom = g_puddleList[i].y + g_puddleList[i].h;
            bool hit = !(pRight < oLeft || pLeft > oRight || pBottom < oTop || pTop > oBottom);
            if (hit) {
                g_slipTimer = SLIP_DURATION;
                // 有水坑隐藏道具则生成出来
                if (g_puddleList[i].hasItem) {
                    SpawnPuddleItem(g_puddleList[i].x + g_puddleList[i].w / 2, g_puddleList[i].y + g_puddleList[i].h / 2);
                    g_puddleList[i].hasItem = false;
                }
            }
        }
    }

    // 加速带碰撞
    for (int i = 0; i < SPEEDBELT_MAX; i++) {
        if (g_speedBeltList[i].active) {
            float oLeft = g_speedBeltList[i].x, oRight = g_speedBeltList[i].x + g_speedBeltList[i].w;
            float oTop = g_speedBeltList[i].y, oBottom = g_speedBeltList[i].y + g_speedBeltList[i].h;
            bool hit = !(pRight < oLeft || pLeft > oRight || pBottom < oTop || pTop > oBottom);
            if (hit) {
                g_beltSpeedBuffTimer = BELT_SPEED_DURATION;
                g_nitroValue += 50.0f;
                if (g_nitroValue > NITRO_MAX) g_nitroValue = NITRO_MAX;
                g_speedBeltList[i].active = false;
                ShowPopup(tipSpeed[GetRandomValue(0, 2)], YELLOW);
            }
        }
    }

    // 路障碰撞
    for (int i = 0; i < ROADBLOCK_MAX; i++) {
        if (g_roadBlockList[i].active) {
            float oLeft = g_roadBlockList[i].x, oRight = g_roadBlockList[i].x + g_roadBlockList[i].w;
            float oTop = g_roadBlockList[i].y, oBottom = g_roadBlockList[i].y + g_roadBlockList[i].h;
            bool hit = !(pRight < oLeft || pLeft > oRight || pBottom < oTop || pTop > oBottom);
            if (hit) {
                // 有护盾或氮气可撞毁路障加分
                if (g_shieldTimer > 0 || g_isNitroActive) {
                    g_roadBlockList[i].active = false;
                    g_comboCount++;
                    g_comboTimer = COMBO_TIMEOUT;
                    g_score += 20 * g_comboCount;
                    continue;
                }
                // 无保护扣血
                TriggerShake();
                g_player.hp--;
                g_roadBlockList[i].active = false;
                g_comboCount = 0;
                g_comboTimer = 0.0f;
                ShowPopup(tipHit[GetRandomValue(0, 2)], RED);
                // 血量为0游戏结束
                if (g_player.hp <= 0) {
                    SaveHighScore();
                    g_state = STATE_OVER;
                }
            }
        }
    }

    // 幽灵障碍物碰撞
    for (int i = 0; i < GHOST_OBSTACLE_MAX; i++) {
        if (g_ghostObsList[i].active) {
            float oLeft = g_ghostObsList[i].x - g_ghostObsList[i].w / 2, oRight = g_ghostObsList[i].x + g_ghostObsList[i].w / 2;
            float oTop = g_ghostObsList[i].y, oBottom = g_ghostObsList[i].y + g_ghostObsList[i].h;
            bool hit = !(pRight < oLeft || pLeft > oRight || pBottom < oTop || pTop > oBottom);
            if (hit) {
                if (g_shieldTimer > 0 || g_isNitroActive) {
                    g_ghostObsList[i].active = false;
                    g_comboCount++;
                    g_comboTimer = COMBO_TIMEOUT;
                    g_score += 15 * g_comboCount;
                    continue;
                }
                TriggerShake();
                g_player.hp--;
                g_ghostObsList[i].active = false;
                g_comboCount = 0;
                g_comboTimer = 0.0f;
                ShowPopup(tipHit[GetRandomValue(0, 2)], RED);
                if (g_player.hp <= 0) {
                    SaveHighScore();
                    g_state = STATE_OVER;
                }
            }
        }
    }

    // 冰雹碰撞
    for (int i = 0; i < HAIL_MAX; i++) {
        if (g_hailList[i].active) {
            float oLeft = g_hailList[i].x, oRight = g_hailList[i].x + 15;
            float oTop = g_hailList[i].y, oBottom = g_hailList[i].y + 15;
            bool hit = !(pRight < oLeft || pLeft > oRight || pBottom < oTop || pTop > oBottom);
            if (hit) {
                g_hailList[i].active = false;
                // 有保护加氮气，无保护减速
                if (g_shieldTimer > 0 || g_isNitroActive) {
                    g_nitroValue += 5.0f;
                    if (g_nitroValue > NITRO_MAX) g_nitroValue = NITRO_MAX;
                }
                else {
                    g_slowTimer = 60.0f;
                    g_isNitroActive = false;
                    g_nitroBurstTimer = 0.0f;
                }
            }
        }
    }

    // 普通障碍物碰撞逻辑
    for (int i = 0; i < MAX_OBSTACLE; i++) {
        if (g_obsList[i].active) {
            float oLeft = g_obsList[i].x - g_obsList[i].w / 2;
            float oRight = g_obsList[i].x + g_obsList[i].w / 2;
            float oTop = g_obsList[i].y, oBottom = g_obsList[i].y + g_obsList[i].h;

            bool hit = true;
            if (pRight < oLeft) hit = false;
            if (pLeft > oRight) hit = false;
            if (pBottom < oTop) hit = false;
            if (pTop > oBottom) hit = false;

            if (hit) {
                // 撞到行人特殊处理：无护盾会中断氮气
                if (g_obsList[i].type == OBJ_PEOPLE) {
                    g_obsList[i].active = false;
                    if (g_isNitroActive && g_shieldTimer <= 0) {
                        g_isNitroActive = false;
                        g_nitroBurstTimer = 0.0f;
                        g_comboCount = 0;
                        g_comboTimer = 0.0f;
                        ShowPopup(tipComboBreak[GetRandomValue(0, 2)], RED);
                        continue;
                    }
                }

                // 有护盾/氮气 撞毁障碍物加分
                if (g_shieldTimer > 0 || g_isNitroActive) {
                    g_obsList[i].active = false;
                    g_comboCount++;
                    g_comboTimer = COMBO_TIMEOUT;

                    // 不同障碍物给不同分数
                    int scoreBonus = 10;
                    if (g_obsList[i].type == OBJ_BUS) scoreBonus = 25;
                    if (g_obsList[i].type == OBJ_STATIC) scoreBonus = 20;
                    g_score += scoreBonus * g_comboCount;

                    // 连击提示
                    if (g_comboCount == 10) ShowPopup(tipCombo10[GetRandomValue(0, 2)], ORANGE);
                    if (g_comboCount == 20) ShowPopup(tipCombo20[GetRandomValue(0, 2)], RED);
                    continue;
                }

                // 无保护扣血重置连击
                TriggerShake();
                g_player.hp--;
                g_obsList[i].active = false;
                g_comboCount = 0;
                g_comboTimer = 0.0f;
                ShowPopup(tipHit[GetRandomValue(0, 2)], RED);

                if (g_player.hp <= 0) {
                    SaveHighScore();
                    g_state = STATE_OVER;
                }
            }
        }
    }

    // 道具碰撞拾取
    for (int i = 0; i < MAX_ITEM; i++) {
        if (g_itemList[i].active) {
            float iLeft = g_itemList[i].x - 18, iRight = g_itemList[i].x + 18;
            float iTop = g_itemList[i].y, iBottom = g_itemList[i].y + 36;

            bool hit = true;
            if (pRight < iLeft) hit = false;
            if (pLeft > iRight) hit = false;
            if (pBottom < iTop) hit = false;
            if (pTop > iBottom) hit = false;

            if (hit) {
                g_itemList[i].active = false;
                // 拾取不同道具触发对应效果
                if (g_itemList[i].type == ITEM_LIFE && g_player.hp < MAX_HP) {
                    g_player.hp++;
                    ShowPopup(tipLife[GetRandomValue(0, 2)], GREEN);
                }
                else if (g_itemList[i].type == ITEM_SCORE) {
                    g_score += 150;
                    ShowPopup(tipScore[GetRandomValue(0, 2)], GOLD);
                }
                else if (g_itemList[i].type == ITEM_SPEED) {
                    g_speedBuffTimer = SPEED_BUFF_DURATION;
                    ShowPopup(tipSpeed[GetRandomValue(0, 2)], SKYBLUE);
                }
                else if (g_itemList[i].type == ITEM_SHIELD) {
                    g_shieldTimer = SHIELD_DURATION;
                    ShowPopup(tipShield[GetRandomValue(0, 2)], PURPLE);
                }
                else if (g_itemList[i].type == ITEM_SLOW) {
                    g_slowTimer = SLOW_DURATION;
                    ShowPopup(tipSlow[GetRandomValue(0, 2)], DARKGRAY);
                }
                else if (g_itemList[i].type == ITEM_NITRO) {
                    g_nitroValue += 25.0f;
                    if (g_nitroValue > NITRO_MAX) g_nitroValue = NITRO_MAX;
                    ShowPopup(tipNitro[GetRandomValue(0, 2)], BLUE);
                }
                else if (g_itemList[i].type == ITEM_MAGNET) {
                    g_magnetTimer = MAGNET_DURATION;
                    ShowPopup(tipMagnet[GetRandomValue(0, 2)], MAGENTA);
                }
            }
        }
    }
}
// ====================== 绘制氮气条 ======================
static void DrawNitroBar() {
    DrawRectangle(20, 260, 150, 20, DARKGRAY);
    Color nitroColor = (g_nitroValue >= NITRO_MAX) ? BLUE : SKYBLUE;
    if (g_nitroValue >= NITRO_MAX && GetRandomValue(0, 1) == 0) {
        nitroColor = ORANGE;
    }
    DrawRectangle(20, 260, (int)(150 * (g_nitroValue / NITRO_MAX)), 20, nitroColor);
    DrawRectangleLines(20, 260, 150, 20, WHITE);
    DrawText("NITRO", 20, 285, 20, WHITE);
    if (g_nitroValue >= NITRO_MAX) {
        DrawText("FULL! PRESS SPACE", 180, 260, 20, ORANGE);
    }
    // 磁铁状态显示
    if (g_magnetTimer > 0) {
        DrawText(TextFormat("MAGNET: %.1fs", g_magnetTimer / 60), 20, 350, 25, MAGENTA);
        DrawText("AUTO COLLECT ACTIVE", 20, 380, 20, MAGENTA);
    }
}

// ====================== 游戏主逻辑更新 ======================
void GameUpdate() {
    // 启动页逻辑
    UpdateSplash();
    // 启动页未结束时，所有游戏逻辑暂停
    if (!g_splashDone) return;

    // ESC键退出游戏
    if (IsKeyPressed(KEY_ESCAPE) && g_state == STATE_PLAYING) {
        g_isPause = !g_isPause;
    }
    if (g_isPause) return;

    // 更新各类计时器
    if (g_flashTimer > 0) g_flashTimer--;
    if (g_popup.timer > 0) g_popup.timer--;
    if (g_shieldTimer > 0) g_shieldTimer--;
    if (g_speedBuffTimer > 0) g_speedBuffTimer--;
    if (g_slowTimer > 0) g_slowTimer--;

    // 连击超时重置
    if (g_comboTimer > 0) g_comboTimer--;
    else g_comboCount = 0;

    UpdatePuddle();
    UpdateSunGlare();
    UpdateGhostObstacle();
    UpdateHail();
    UpdateRoadBlock();
    UpdateSpeedBelt();

    // 更新所有场景系统
    UpdateExhaust();
    UpdateShake();
    UpdateRain();
    UpdateWeather();
    UpdateNitro();
    UpdateSpeedLines();
    UpdateGameBg(); // 单张图循环+氮气特效

    // 难度选择界面逻辑
    if (g_state == (GameState)STATE_DIFFICULTY) {
        if (IsKeyPressed(KEY_LEFT) && g_selectedDifficulty > 0) g_selectedDifficulty--;
        if (IsKeyPressed(KEY_RIGHT) && g_selectedDifficulty < 2) g_selectedDifficulty++;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            ApplyDifficulty();
            g_state = STATE_SELECT_CAR;
        }
        return;
    }

    // 车辆选择界面逻辑
    if (g_state == STATE_SELECT_CAR) {
        if (IsKeyPressed(KEY_LEFT) && g_selectedCar > 0) g_selectedCar--;
        if (IsKeyPressed(KEY_RIGHT) && g_selectedCar < CAR_COUNT - 1) g_selectedCar++;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            g_state = STATE_COUNTDOWN;
            g_countdownTimer = 180;
            g_countdownNum = 3;
            InitPlayer();
        }
        return;
    }

    // 倒计时界面逻辑
    if (g_state == STATE_COUNTDOWN) {
        g_countdownTimer--;
        g_countdownNum = (int)(g_countdownTimer / 60) + 1;
        if (g_countdownTimer <= 0) {
            g_state = STATE_PLAYING;
        }
        return;
    }

    // 游戏结束界面逻辑
    if (g_state == STATE_OVER) {
        if (IsKeyPressed(KEY_R)) {
            GameInit();
        }
        return;
    }

    // 游戏难度随时间递增
    if (g_obsBaseSpd < 12) g_obsBaseSpd += 0.004f;
    UpdatePlayer();
    g_score++;

    // 生成动态障碍物和道具
    g_spawnTimer++;
    if (g_spawnTimer >= g_spawnGap) {
        SpawnDynamicObs();
        if (GetRandomValue(0, 3) == 0) SpawnItem();
        g_spawnTimer = 0;
    }

    // 生成静态路障
    g_staticSpawnTimer++;
    if (g_staticSpawnTimer >= STATIC_SPAWN_INTERVAL) {
        SpawnStaticObs();
        g_staticSpawnTimer = 0;
    }

    // 最终更新与碰撞检测
    UpdateObs();
    UpdateItems();
    CheckCollision();
}

// ====================== 游戏主绘制 ======================
void GameDraw() {
    BeginDrawing();
    ClearBackground(BLACK);

    // 先绘制启动页（优先级最高）
    DrawSplash();
    // 启动页未结束时，不绘制任何其他内容
    if (!g_splashDone) {
        EndDrawing();
        return;
    }

    // 氮气模式开启2D相机缩放
// 【豆包AI辅助】：2D相机系统、视图缩放特效
    Camera2D nitroCamera = { 0 };
    if (g_isNitroActive) {
        nitroCamera.offset.x = (float)SCREEN_WIDTH / 2.0f;
        nitroCamera.offset.y = (float)SCREEN_HEIGHT / 2.0f;
        nitroCamera.target.x = (float)SCREEN_WIDTH / 2.0f;
        nitroCamera.target.y = (float)SCREEN_HEIGHT / 2.0f;
        nitroCamera.rotation = 0.0f;
        nitroCamera.zoom = NITRO_CAMERA_ZOOM;
        BeginMode2D(nitroCamera);
    }

    // 难度选择界面绘制
    if (g_state == (GameState)STATE_DIFFICULTY) {
        // 绘制共用菜单背景
        float scaleX = (float)SCREEN_WIDTH / g_menuBgTex.width;
        float scaleY = (float)SCREEN_HEIGHT / g_menuBgTex.height;
        float scale = (scaleX > scaleY) ? scaleX : scaleY;
        float drawX = (SCREEN_WIDTH - g_menuBgTex.width * scale) / 2.0f;
        float drawY = (SCREEN_HEIGHT - g_menuBgTex.height * scale) / 2.0f;
        DrawTextureEx(g_menuBgTex, { drawX, drawY }, 0.0f, scale, Fade(WHITE, BG_DARK_ALPHA));

        const char* diffs[] = { "EASY", "NORMAL", "HARD" };
        Color diffColors[] = { GREEN, GOLD, RED };
        const char* title = "SELECT DIFFICULTY";
        int titleWidth = MeasureText(title, 60);
        DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, 100, 60, WHITE);

        const char* tip = "LEFT/RIGHT ARROW TO SWITCH | ENTER TO CONFIRM";
        int tipWidth = MeasureText(tip, 30);
        DrawText(tip, (SCREEN_WIDTH - tipWidth) / 2, 180, 30, LIGHTGRAY);

        char diffText[64];
        sprintf_s(diffText, sizeof(diffText), "CURRENT: %s", diffs[g_selectedDifficulty]);
        int diffWidth = MeasureText(diffText, 45);
        DrawText(diffText, (SCREEN_WIDTH - diffWidth) / 2, 260, 45, diffColors[g_selectedDifficulty]);

        const char* easyScore = TextFormat("EASY BEST: %d", g_highScore[DIFF_EASY]);
        const char* normalScore = TextFormat("NORMAL BEST: %d", g_highScore[DIFF_NORMAL]);
        const char* hardScore = TextFormat("HARD BEST: %d", g_highScore[DIFF_HARD]);
        DrawText(easyScore, 50, 400, 35, GREEN);
        DrawText(normalScore, (SCREEN_WIDTH - MeasureText(normalScore, 35)) / 2, 400, 35, GOLD);
        DrawText(hardScore, SCREEN_WIDTH - MeasureText(hardScore, 35) - 50, 400, 35, RED);
    }

    // 车辆选择界面绘制
    else if (g_state == STATE_SELECT_CAR) {
        // 绘制相同的菜单背景
        float scaleX = (float)SCREEN_WIDTH / g_menuBgTex.width;
        float scaleY = (float)SCREEN_HEIGHT / g_menuBgTex.height;
        float scale = (scaleX > scaleY) ? scaleX : scaleY;
        float drawX = (SCREEN_WIDTH - g_menuBgTex.width * scale) / 2.0f;
        float drawY = (SCREEN_HEIGHT - g_menuBgTex.height * scale) / 2.0f;
        DrawTextureEx(g_menuBgTex, { drawX, drawY }, 0.0f, scale, Fade(WHITE, BG_DARK_ALPHA));

        const char* title = "SELECT CAR";
        int titleWidth = MeasureText(title, 60);
        DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, 150, 60, WHITE);

        const char* tip = "LEFT/RIGHT ARROW TO SWITCH | ENTER TO START";
        int tipWidth = MeasureText(tip, 30);
        DrawText(tip, (SCREEN_WIDTH - tipWidth) / 2, 250, 30, LIGHTGRAY);

        const char* diffs[] = { "EASY", "NORMAL", "HARD" };
        char highText[64];
        sprintf_s(highText, sizeof(highText), "%s BEST: %d", diffs[g_selectedDifficulty], g_highScore[g_selectedDifficulty]);
        DrawText(highText, 20, 20, 30, GOLD);

        float cx = (float)SCREEN_WIDTH / 2.0f, cy = (float)SCREEN_HEIGHT / 2.0f;
        for (int i = 0; i < CAR_COUNT; i++) {
            float ox = (float)(i - g_selectedCar) * 300.0f;
            float sc = (i == g_selectedCar) ? 1.3f : 0.8f;
            Color tint = (i == g_selectedCar) ? WHITE : Fade(GRAY, 0.6f);

            Rectangle rect;
            rect.x = cx + ox - 75 * sc;
            rect.y = cy + 80 - 90 * sc;  
            rect.width = 150 * sc;
            rect.height = 180 * sc;
            DrawRectangleLinesEx(rect, 4, GOLD);

            DrawTextureAutoScale(carTextures[i], cx + ox, cy + 80, 150 * sc, 180 * sc, tint);
        }
    }
    // 倒计时界面绘制
    else if (g_state == STATE_COUNTDOWN) {
        DrawGameBg(); // 倒计时显示游戏背景

        const char* weatherStr[] = { "SUNNY", "RAINY", "FOGGY", "HAIL" };
        Color weatherColor[] = { YELLOW, SKYBLUE, LIGHTGRAY, SKYBLUE };
        char weatherText[32];
        sprintf_s(weatherText, sizeof(weatherText), "INIT WEATHER: %s", weatherStr[g_currentWeather]);
        int weatherWidth = MeasureText(weatherText, 30);
        DrawText(weatherText, (SCREEN_WIDTH - weatherWidth) / 2, 100, 30, weatherColor[g_currentWeather]);

        char countText[8];
        sprintf_s(countText, sizeof(countText), "%d", g_countdownNum > 0 ? g_countdownNum : 0);
        if (g_countdownNum <= 0) strcpy_s(countText, "GO!");

        int fontSize = g_countdownNum > 0 ? 200 : 150;
        Vector2 textSize = MeasureTextEx(GetFontDefault(), countText, (float)fontSize, 2);
        DrawText(countText, (int)((SCREEN_WIDTH - textSize.x) / 2), (int)((SCREEN_HEIGHT - textSize.y) / 2), fontSize, g_countdownNum > 0 ? WHITE : GOLD);
    }
    // 游戏结束界面绘制
    else if (g_state == STATE_OVER) {
        ClearBackground(RED);
        const char* title = "GAME OVER";
        int titleWidth = MeasureText(title, 70);
        DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, 320, 70, WHITE);

        const char* diffs[] = { "EASY", "NORMAL", "HARD" };
        char scoreText[64], highText[64];
        sprintf_s(scoreText, sizeof(scoreText), "%s SCORE: %d", diffs[g_selectedDifficulty], g_score);
        sprintf_s(highText, sizeof(highText), "%s BEST: %d", diffs[g_selectedDifficulty], g_highScore[g_selectedDifficulty]);

        int scoreWidth = MeasureText(scoreText, 40);
        DrawText(scoreText, (SCREEN_WIDTH - scoreWidth) / 2, 440, 40, WHITE);

        Color highColor = (g_score >= g_highScore[g_selectedDifficulty]) ? GOLD : LIGHTGRAY;
        int highWidth = MeasureText(highText, 35);
        DrawText(highText, (SCREEN_WIDTH - highWidth) / 2, 500, 35, highColor);

        if (g_score >= g_highScore[g_selectedDifficulty]) {
            const char* newRecord = "NEW RECORD!";
            int recordWidth = MeasureText(newRecord, 30);
            DrawText(newRecord, (SCREEN_WIDTH - recordWidth) / 2, 550, 30, YELLOW);
        }

        const char* restartTip = "Press R to Restart";
        int restartWidth = MeasureText(restartTip, 35);
        DrawText(restartTip, (SCREEN_WIDTH - restartWidth) / 2, 600, 35, LIGHTGRAY);
    }
    // 主游戏界面绘制
    else {
        DrawGameBg(); // 单张图循环+氮气变暗

        Rectangle border;
        border.x = 0;
        border.y = 0;
        border.width = SCREEN_WIDTH;
        border.height = SCREEN_HEIGHT;

        // 绘制道路和车道线
        if (g_isNitroActive) {
            int borderWidth = GetRandomValue(10, 15);
            Color borderColor = (GetRandomValue(0, 1) == 0) ? ORANGE : RED;
            DrawRectangleLinesEx(border, (float)borderWidth, Fade(borderColor, 0.9f));
        }
        else if (g_shieldTimer > 0) {
            DrawRectangleLinesEx(border, 8, PURPLE);
        }

        // 绘制速度线和眩光
        DrawSpeedLines();
        DrawSunGlare();

        // 绘制尾气特效
        for (int i = 0; i < EXHAUST_MAX; i++) {
            if (g_exhaust[i].life > 0) {
                DrawRectangle((int)(g_exhaust[i].x + g_shakeOffsetX), (int)(g_exhaust[i].y + g_shakeOffsetY), 6, 6, g_exhaust[i].col);
            }
        }

        // 绘制水坑、路障、加速带
        for (int i = 0; i < PUDDLE_MAX; i++) {
            if (g_puddleList[i].active) {
                DrawTextureAutoScale(
                    puddleTexture,
                    g_puddleList[i].x + g_puddleList[i].w / 2 + g_shakeOffsetX,
                    g_puddleList[i].y + g_puddleList[i].h / 2 + g_shakeOffsetY,
                    g_puddleList[i].w,
                    g_puddleList[i].h,
                    Fade(SKYBLUE, 0.4f)
                );
            }
        }
        for (int i = 0; i < ROADBLOCK_MAX; i++) {
            if (g_roadBlockList[i].active) {
                DrawTextureAutoScale(
                    roadblockTexture,
                    g_roadBlockList[i].x + g_roadBlockList[i].w / 2 + g_shakeOffsetX,
                    g_roadBlockList[i].y + g_roadBlockList[i].h / 2 + g_shakeOffsetY,
                    g_roadBlockList[i].w,
                    g_roadBlockList[i].h,
                    ORANGE
                );
            }
        }
        for (int i = 0; i < SPEEDBELT_MAX; i++) {
            if (g_speedBeltList[i].active) {
                DrawRectangle(
                    (int)(g_speedBeltList[i].x + g_shakeOffsetX),
                    (int)(g_speedBeltList[i].y + g_shakeOffsetY),
                    (int)g_speedBeltList[i].w,
                    (int)g_speedBeltList[i].h,
                    Fade(YELLOW, 0.5f)
                );
            }
        }

        // 绘制障碍物
        for (int i = 0; i < GHOST_OBSTACLE_MAX; i++) {
            if (g_ghostObsList[i].active) {
                float dist = fabsf(g_ghostObsList[i].y - g_player.y);
                float alpha = dist < 200.0f ? 1.0f : 0.3f;

                DrawTextureAutoScale(
                    ghostTexture,
                    g_ghostObsList[i].x + g_shakeOffsetX,
                    g_ghostObsList[i].y + g_ghostObsList[i].h / 2 + g_shakeOffsetY,
                    g_ghostObsList[i].w,
                    g_ghostObsList[i].h,
                    Fade(WHITE, alpha)
                );
            }
        }
        DrawHail();
        for (int i = 0; i < MAX_OBSTACLE; i++) {
            if (g_obsList[i].active) {
                Color c = WHITE;
                if (g_obsList[i].type == OBJ_CAT) c = ORANGE;
                if (g_obsList[i].type == OBJ_PEOPLE) c = YELLOW;
                if (g_obsList[i].type == OBJ_BIKE) c = GREEN;
                if (g_obsList[i].type == OBJ_BUS) c = BLUE;
                if (g_obsList[i].type == OBJ_STATIC) c = DARKGRAY;

                if (g_fogAlpha > 0.01f) {
                    float alpha = 1.0f;
                    if (g_obsList[i].y < 300) alpha = g_obsList[i].y / 300.0f;
                    c.a = (unsigned char)(alpha * 255 * (1 - g_fogAlpha * 0.5f));
                }

                DrawTextureAutoScale(
                    obstacleTextures[g_obsList[i].type],
                    g_obsList[i].x + g_shakeOffsetX,
                    g_obsList[i].y + g_obsList[i].h / 2 + g_shakeOffsetY,
                    g_obsList[i].w,
                    g_obsList[i].h,
                    c
                );
            }
        }

        // 绘制道具
        for (int i = 0; i < MAX_ITEM; i++) {
            if (g_itemList[i].active) {
                Color c = GREEN;
                if (g_itemList[i].type == ITEM_SCORE) c = GOLD;
                if (g_itemList[i].type == ITEM_SPEED) c = SKYBLUE;
                if (g_itemList[i].type == ITEM_SHIELD) c = PURPLE;
                if (g_itemList[i].type == ITEM_SLOW) c = DARKGRAY;
                if (g_itemList[i].type == ITEM_NITRO) c = BLUE;
                if (g_itemList[i].type == ITEM_MAGNET) c = MAGENTA;

                if (g_fogAlpha > 0.01f) {
                    float alpha = 1.0f;
                    if (g_itemList[i].y < 300) alpha = g_itemList[i].y / 300.0f;
                    c.a = (unsigned char)(alpha * 255 * (1 - g_fogAlpha * 0.5f));
                }

                DrawTextureAutoScale(
                    itemTextures[g_itemList[i].type],
                    g_itemList[i].x + g_shakeOffsetX,
                    g_itemList[i].y + 18 + g_shakeOffsetY,
                    36,
                    36,
                    c
                );
            }
        }

        // 绘制玩家车辆
        Color carTint = WHITE;
        if (g_isNitroActive) {
            if (GetRandomValue(0, 1) == 0) {
                DrawRectangleLines(
                    (int)(g_player.x - 52.5f + g_shakeOffsetX),
                    (int)(g_player.y + g_shakeOffsetY),
                    105,
                    130,
                    ORANGE
                );
            }
        }
        else if (g_shieldTimer > 0) {
            DrawCircle(
                (int)(g_player.x + g_shakeOffsetX),
                (int)(g_player.y + 65 + g_shakeOffsetY),
                65,
                Fade(PURPLE, 0.3f)
            );
        }

        DrawTextureAutoScale(
            carTextures[g_player.carId],
            g_player.x + g_shakeOffsetX,
            g_player.y + 65 + g_shakeOffsetY,
            105,
            130,
            WHITE
        );

        // 绘制天气特效
        DrawRain();
        DrawFog();

        // 绘制UI界面
        Color uiColor = WHITE;
        const char* diffs[] = { "EASY", "NORMAL", "HARD" };
        const char* weatherStr[] = { "SUNNY", "RAINY", "FOGGY", "HAIL" };
        Color weatherColor[] = { YELLOW, SKYBLUE, LIGHTGRAY, SKYBLUE };

        DrawText(TextFormat("HP: %d", g_player.hp), 20, 20, 30, uiColor);
        DrawText(TextFormat("SCORE: %d", g_score), 20, 60, 30, uiColor);
        DrawText(TextFormat("%s BEST: %d", diffs[g_selectedDifficulty], g_highScore[g_selectedDifficulty]), 20, 100, 30, GOLD);
        DrawText(TextFormat("WEATHER: %s", weatherStr[g_currentWeather]), 20, 140, 25, weatherColor[g_currentWeather]);

        if (g_speedBuffTimer > 0) DrawText("SPEED UP!", 20, 170, 25, BLUE);
        if (g_shieldTimer > 0) DrawText(TextFormat("SHIELD: %.1fs", g_shieldTimer / 60), 20, 200, 25, PURPLE);
        if (g_slowTimer > 0) DrawText(TextFormat("SLOW: %.1fs", g_slowTimer / 60), 20, 230, 25, DARKGRAY);

        DrawNitroBar();

        if (g_comboCount > 0) {
            Color comboColor = g_comboCount >= 20 ? RED : (g_comboCount >= 10 ? ORANGE : YELLOW);
            DrawText(TextFormat("COMBO: %d", g_comboCount), 20, 320, 25, comboColor);
        }
        
        // 绘制当前速度
        float currentSpeed = g_obsBaseSpd * 10.0f;
        if (g_isNitroActive) currentSpeed *= 3.5f;
        if (g_beltSpeedBuffTimer > 0) currentSpeed *= 2.0f;
        if (g_speedBuffTimer > 0) currentSpeed *= 1.5f;
        Color speedColor = g_isNitroActive ? RED : WHITE;
        DrawText(TextFormat("SPEED: %.0f KM/H", currentSpeed), SCREEN_WIDTH - 280, 20, 30, speedColor);

        // 绘制弹窗提示
        if (g_popup.timer > 0) {
            int popupWidth = MeasureText(g_popup.text, 70);
            DrawText(g_popup.text, (SCREEN_WIDTH - popupWidth) / 2, 300, 70, g_popup.color);
        }
        if (g_flashTimer > 0) DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, 0.3f));
        if (g_isPause) {
            const char* pauseTip = "PAUSED - ESC to Resume";
            int pauseWidth = MeasureText(pauseTip, 50);
            DrawText(pauseTip, (SCREEN_WIDTH - pauseWidth) / 2, 350, 50, WHITE);
        }
    }

    if (g_isNitroActive) {
        EndMode2D();
    }

    EndDrawing();
}