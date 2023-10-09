#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "/opt/homebrew/Cellar/raylib/4.5.0/include/raylib.h"
#include "/opt/homebrew/Cellar/raylib/4.5.0/include/raymath.h"

#define MAX_AGENTS 16
#define MAX_FOODS 8

#define GRID_SIZE 32
#define SIZE_X 32
#define SIZE_Y 24

typedef enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

typedef enum AgentType
{
    PLAYER,
    ENEMY
} AgentType;

typedef enum
{
    PAUSED,
    PLAY,
    MENU,
    GAME_OVER
} GameState;

typedef enum
{
    WANDER,
    ATTACK
} Behaviour;

typedef struct
{
    Vector2 position;
    bool alive;
} Food;

typedef struct Tile
{
    int tileId;
    bool solid;
} Tile;

typedef struct Attack
{
    float radius;
    bool active;
    float timer;
    float cooldown;
    float dragTime;
} Attack;

typedef struct Agent
{
    AgentType type;
    Vector2 position;
    int health;
    int maxHealth;
    float speed;
    float invincible;
    Attack attack;
    Direction direction;
    Texture2D sprite;
} Agent;

typedef struct
{
    int enemyIndex;
    int count;
} Wav;

typedef struct
{
    int origin;
    float variance;
    float speed;
} BobStat;