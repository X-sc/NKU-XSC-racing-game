#include "game_def.h"

// 全局游戏状态与核心变量
GameState g_state = STATE_MENU;
int       g_score = 0;
float     g_flashTimer = 0.0f;

// 动态障碍物生成（随分数变密）
int       g_spawnTimer = 0;
int       g_spawnGap = 100;

// 静态障碍物生成（固定密度）
int       g_staticSpawnTimer = 0;
#define STATIC_SPAWN_INTERVAL 220

float     g_obsBaseSpd = 6.0f;
float     g_laneWidth;
float     g_laneCenter[LANE_COUNT];

Player g_player;
Obstacle g_obsList[MAX_OBSTACLE];
Item     g_itemList[MAX_ITEM];

// 玩家速度参数
#define PLAYER_BASE_SPEED 6.0f
#define PLAYER_SPEED_MAX  8.5f
float g_playerSpeed = PLAYER_BASE_SPEED;
float g_speedBuffTimer = 0.0f;
#define SPEED_BUFF_DURATION 180.0f

// 各类型障碍物基础速度
#define SPD_BIKE    7.2f
#define SPD_CAT     6.5f
#define SPD_BUS     5.8f
#define SPD_PEOPLE  5.0f
#define SPD_STATIC  4.5f

// 初始化车道坐标
static void InitLanePos()
{
    g_laneWidth = (float)ROAD_WIDTH / LANE_COUNT;
    g_laneCenter[0] = ROAD_X + g_laneWidth * 0 + g_laneWidth / 2;
    g_laneCenter[1] = ROAD_X + g_laneWidth * 1 + g_laneWidth / 2;
    g_laneCenter[2] = ROAD_X + g_laneWidth * 2 + g_laneWidth / 2;
}

// 初始化玩家状态
static void InitPlayer()
{
    g_player.x = ROAD_X + ROAD_WIDTH / 2.0f;
    g_player.y = 680.0f;
    g_player.hp = MAX_HP;
    g_speedBuffTimer = 0.0f;
}

// 初始化障碍物数组
static void InitObstacles()
{
    for (int i = 0; i < MAX_OBSTACLE; i++)
        g_obsList[i].active = false;
}

// 初始化道具数组
static void InitItems()
{
    for (int i = 0; i < MAX_ITEM; i++)
        g_itemList[i].active = false;
}

// 生成动态障碍物（小猫/行人/自行车/公交）
static void SpawnDynamicObstacle()
{
    for (int i = 0; i < MAX_OBSTACLE; i++)
    {
        if (!g_obsList[i].active)
        {
            int rnd = GetRandomValue(0, 3);
            ObstacleType type = (ObstacleType)rnd;
            g_obsList[i].type = type;

            // 按类型设置尺寸
            if (type == OBJ_CAT)
            {
                g_obsList[i].w = 45; g_obsList[i].h = 55;
            }
            else if (type == OBJ_PEOPLE)
            {
                g_obsList[i].w = 35; g_obsList[i].h = 70;
            }
            else if (type == OBJ_BIKE)
            {
                g_obsList[i].w = 50; g_obsList[i].h = 75;
            }
            else if (type == OBJ_BUS)
            {
                g_obsList[i].w = 85; g_obsList[i].h = 95;
            }

            // 随机生成位置
            int minX = ROAD_X + (int)(g_obsList[i].w / 2);
            int maxX = ROAD_X + ROAD_WIDTH - (int)(g_obsList[i].w / 2);
            g_obsList[i].x = (float)GetRandomValue(minX, maxX);
            g_obsList[i].y = -g_obsList[i].h;

            // 【豆包AI辅助】分层速度设计：保持不同障碍物的快慢关系
            float addSpeed = g_obsBaseSpd - 6.0f;
            if (type == OBJ_BIKE)
                g_obsList[i].spdY = SPD_BIKE + addSpeed;
            else if (type == OBJ_CAT)
                g_obsList[i].spdY = SPD_CAT + addSpeed;
            else if (type == OBJ_BUS)
                g_obsList[i].spdY = SPD_BUS + addSpeed;
            else
                g_obsList[i].spdY = SPD_PEOPLE + addSpeed;

            g_obsList[i].spdX = (float)GetRandomValue(-15, 15) / 10.0f;
            g_obsList[i].active = true;
            break;
        }
    }
}

// 生成静态障碍物
static void SpawnStaticObstacle()
{
    for (int i = 0; i < MAX_OBSTACLE; i++)
    {
        if (!g_obsList[i].active)
        {
            g_obsList[i].type = OBJ_STATIC;
            g_obsList[i].w = 65;
            g_obsList[i].h = 110;

            float obsHalfW = g_obsList[i].w / 2.0f;
            int side = GetRandomValue(0, 1);
            float randX;

            if (side == 0)
            {
                float leftMin = ROAD_X + obsHalfW;
                float leftMax = ROAD_X + g_laneWidth - obsHalfW;
                randX = GetRandomValue((int)leftMin, (int)leftMax);
            }
            else
            {
                float rightMin = ROAD_X + g_laneWidth * 2 + obsHalfW;
                float rightMax = ROAD_X + ROAD_WIDTH - obsHalfW;
                randX = GetRandomValue((int)rightMin, (int)rightMax);
            }

            g_obsList[i].x = randX;
            g_obsList[i].y = -g_obsList[i].h;
            g_obsList[i].spdY = SPD_STATIC;
            g_obsList[i].spdX = 0.0f;
            g_obsList[i].active = true;
            break;
        }
    }
}

// 生成随机道具
static void SpawnItem()
{
    for (int i = 0; i < MAX_ITEM; i++)
    {
        if (!g_itemList[i].active)
        {
            int rnd = GetRandomValue(0, 2);
            g_itemList[i].type = (ItemType)rnd;
            g_itemList[i].w = 40;
            g_itemList[i].h = 40;
            g_itemList[i].x = (float)GetRandomValue(ROAD_X + 20, ROAD_X + ROAD_WIDTH - 20);
            g_itemList[i].y = -50.0f;
            g_itemList[i].spdY = g_obsBaseSpd * 0.8f;
            g_itemList[i].active = true;
            break;
        }
    }
}

// 全局游戏初始化
void GameInit()
{
    InitLanePos();
    g_state = STATE_MENU;
    g_score = 0;
    g_flashTimer = 0.0f;

    g_spawnTimer = 0;
    g_staticSpawnTimer = 0;
    g_spawnGap = 100;
    g_obsBaseSpd = 6.0f;

    InitPlayer();
    InitObstacles();
    InitItems();
}

// 更新玩家移动与状态
static void UpdatePlayer()
{
    // 玩家速度随全局难度同步提升
    float diff = g_obsBaseSpd - 6.0f;
    float curBaseSpeed = PLAYER_BASE_SPEED + diff * 0.25f;
    if (curBaseSpeed > PLAYER_SPEED_MAX) curBaseSpeed = PLAYER_SPEED_MAX;

    // 加速道具效果
    if (g_speedBuffTimer > 0.0f)
    {
        g_speedBuffTimer -= 1.0f;
        curBaseSpeed *= 1.8f;
    }

    // 玩家移动控制
    if (IsKeyDown(KEY_LEFT))  g_player.x -= curBaseSpeed;
    if (IsKeyDown(KEY_RIGHT)) g_player.x += curBaseSpeed;
    if (IsKeyDown(KEY_W)) g_player.y -= 5.0f;
    if (IsKeyDown(KEY_S)) g_player.y += 5.0f;

    // 边界限制
    if (g_player.y < 350.0f) g_player.y = 350.0f;
    if (g_player.y > 820.0f) g_player.y = 820.0f;
    if (g_player.x < ROAD_X + 40)  g_player.x = ROAD_X + 40;
    if (g_player.x > ROAD_X + ROAD_WIDTH - 40) g_player.x = ROAD_X + ROAD_WIDTH - 40;
}

// 【豆包AI辅助】更新所有障碍物
static void UpdateObstacles()
{
    for (int i = 0; i < MAX_OBSTACLE; i++)
    {
        if (g_obsList[i].active)
        {
            g_obsList[i].y += g_obsList[i].spdY;
            if (g_obsList[i].type != OBJ_STATIC)
                g_obsList[i].x += g_obsList[i].spdX;

            // 动态障碍物边缘反弹
            if (g_obsList[i].type != OBJ_STATIC)
            {
                float leftBd = ROAD_X + g_obsList[i].w / 2;
                float rightBd = ROAD_X + ROAD_WIDTH - g_obsList[i].w / 2;
                if (g_obsList[i].x <= leftBd || g_obsList[i].x >= rightBd)
                    g_obsList[i].spdX *= -1;
            }

            if (g_obsList[i].y > SCREEN_HEIGHT)
                g_obsList[i].active = false;
        }
    }
}

// 更新所有道具
static void UpdateItems()
{
    for (int i = 0; i < MAX_ITEM; i++)
    {
        if (g_itemList[i].active)
        {
            g_itemList[i].y += g_itemList[i].spdY;
            if (g_itemList[i].y > SCREEN_HEIGHT)
                g_itemList[i].active = false;
        }
    }
}

// 【豆包AI辅助】AABB轴对齐包围盒碰撞检测
static void CheckCollision()
{
    float pL = g_player.x - 40.0f;
    float pR = g_player.x + 40.0f;
    float pT = g_player.y;
    float pB = g_player.y + 100.0f;

    // 障碍物碰撞
    for (int i = 0; i < MAX_OBSTACLE; i++)
    {
        if (g_obsList[i].active)
        {
            float oL = g_obsList[i].x - g_obsList[i].w / 2;
            float oR = g_obsList[i].x + g_obsList[i].w / 2;
            float oT = g_obsList[i].y;
            float oB = g_obsList[i].y + g_obsList[i].h;

            bool hit = !(pR < oL || pL > oR || pB < oT || pT > oB);
            if (hit)
            {
                g_player.hp--;
                g_obsList[i].active = false;
                if (g_player.hp > 0) g_flashTimer = 12.0f;
                if (g_player.hp <= 0) g_state = STATE_OVER;
            }
        }
    }

    // 道具拾取
    for (int i = 0; i < MAX_ITEM; i++)
    {
        if (g_itemList[i].active)
        {
            float iL = g_itemList[i].x - 20.0f;
            float iR = g_itemList[i].x + 20.0f;
            float iT = g_itemList[i].y;
            float iB = g_itemList[i].y + 40.0f;

            bool hit = !(pR < iL || pL > iR || pB < iT || pT > iB);
            if (hit)
            {
                g_itemList[i].active = false;
                if (g_itemList[i].type == ITEM_HEART && g_player.hp < MAX_HP)
                    g_player.hp++;
                if (g_itemList[i].type == ITEM_STAR)
                    g_score += 150;
                if (g_itemList[i].type == ITEM_SPEED)
                    g_speedBuffTimer = SPEED_BUFF_DURATION;
            }
        }
    }
}

// 游戏主逻辑更新
void GameUpdate()
{
    if (g_flashTimer > 0) g_flashTimer -= 1.0f;

    if (g_state == STATE_MENU)
    {
        if (IsKeyPressed(KEY_ENTER))
            g_state = STATE_PLAY;
        return;
    }

    if (g_state == STATE_OVER)
    {
        if (IsKeyPressed(KEY_R))
            GameInit();
        return;
    }

    // 【豆包AI辅助】动态难度调整系统
    if (g_obsBaseSpd < 12.0f)
        g_obsBaseSpd += 0.004f;

    // 动态障碍物生成间隔随分数缩短
    if (g_score < 600)
        g_spawnGap = 100 - (int)(g_score / 15);
    else
        g_spawnGap = 35;
    if (g_spawnGap < 35) g_spawnGap = 35;

    UpdatePlayer();
    g_score++;

    // 生成动态障碍物和道具
    g_spawnTimer++;
    if (g_spawnTimer >= g_spawnGap)
    {
        SpawnDynamicObstacle();
        if (GetRandomValue(0, 3) == 0) SpawnItem();
        g_spawnTimer = 0;
    }

    // 生成静态障碍物
    g_staticSpawnTimer++;
    if (g_staticSpawnTimer >= STATIC_SPAWN_INTERVAL)
    {
        SpawnStaticObstacle();
        g_staticSpawnTimer = 0;
    }

    UpdateObstacles();
    UpdateItems();
    CheckCollision();
}

// 游戏绘制【豆包AI修改部分逻辑】
void GameDraw()
{
    BeginDrawing();

    if (g_state == STATE_MENU)
    {
        ClearBackground(DARKGREEN);
        DrawText("START", 550, 320, 70, WHITE);
        DrawText("Press ENTER to Start", 500, 440, 35, LIGHTGRAY);
    }
    else if (g_state == STATE_OVER)
    {
        ClearBackground(RED);
        DrawText("GAME OVER", 500, 320, 70, WHITE);
        DrawText(TextFormat("Score: %d", g_score), 530, 440, 40, WHITE);
        DrawText("Press R to Restart", 490, 540, 35, LIGHTGRAY);
    }
    else
    {
        Color bg = { 210, 230, 215, 255 };
        ClearBackground(bg);

        // 绘制马路与车道线
        DrawRectangle(ROAD_X, 0, ROAD_WIDTH, SCREEN_HEIGHT, GRAY);
        DrawRectangle((int)(ROAD_X + g_laneWidth - 2), 0, 4, SCREEN_HEIGHT, WHITE);
        DrawRectangle((int)(ROAD_X + g_laneWidth * 2 - 2), 0, 4, SCREEN_HEIGHT, WHITE);

        // 绘制障碍物
        for (int i = 0; i < MAX_OBSTACLE; i++)
        {
            if (g_obsList[i].active)
            {
                Color c = WHITE;
                if (g_obsList[i].type == OBJ_CAT)      c = ORANGE;
                if (g_obsList[i].type == OBJ_PEOPLE)   c = YELLOW;
                if (g_obsList[i].type == OBJ_BIKE)     c = GREEN;
                if (g_obsList[i].type == OBJ_BUS)      c = BLUE;
                if (g_obsList[i].type == OBJ_STATIC)    c = DARKGRAY;

                DrawRectangle((int)(g_obsList[i].x - g_obsList[i].w / 2),
                    (int)g_obsList[i].y,
                    (int)g_obsList[i].w,
                    (int)g_obsList[i].h, c);
            }
        }

        // 绘制道具
        for (int i = 0; i < MAX_ITEM; i++)
        {
            if (g_itemList[i].active)
            {
                Color c = GREEN;
                if (g_itemList[i].type == ITEM_STAR) c = GOLD;
                if (g_itemList[i].type == ITEM_SPEED) c = SKYBLUE;

                DrawRectangle((int)(g_itemList[i].x - g_itemList[i].w / 2),
                    (int)g_itemList[i].y,
                    (int)g_itemList[i].w,
                    (int)g_itemList[i].h, c);
            }
        }

        // 绘制玩家与UI
        DrawRectangle((int)(g_player.x - 40), (int)g_player.y, 80, 100, PURPLE);
        DrawText(TextFormat("HP: %d", g_player.hp), 20, 20, 30, BLACK);
        DrawText(TextFormat("Score: %d", g_score), 20, 60, 30, BLACK);
        if (g_speedBuffTimer > 0)
            DrawText("SPEED UP!", 20, 100, 30, BLUE);

        // 受伤红闪效果
        if (g_flashTimer > 0)
        {
            Color flash = { 200, 0, 0, 80 };
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, flash);
        }
    }

    EndDrawing();
}