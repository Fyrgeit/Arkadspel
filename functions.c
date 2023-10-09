#include "structs.c"

int Dir2Deg(Direction dir)
{
    if (dir == DOWN) return 0;
    if (dir == RIGHT) return 90;
    if (dir == UP) return 180;
    if (dir == LEFT) return 270;
    return 0;
}

void AddAgent(Agent agents[], Agent newAgent, Vector2 position)
{
    newAgent.position = position;
    
    for (int i = 0; i < MAX_AGENTS; i++)
    {
        if (agents[i].health <= 0)
        {
            agents[i] = newAgent;
            return;
        }
    }
}

void AddFood(Food foods[], Food newFood)
{
    for (int i = 0; i < MAX_FOODS; i++)
    {
        if (!foods[i].alive)
        {
            foods[i] = newFood;
            return;
        }
    }
}

void AgentCollision(Agent *agentPtr, Vector2 movement, Tile* map)
{
    //Vertical
    (*agentPtr).position.y += movement.y;
    for (int y = 0; y < SIZE_Y; y++)
    {
        for (int x = 0; x < SIZE_X; x++)
        {
            if (map[y * SIZE_X + x].solid)
            {
                if ((*agentPtr).position.x > (x - 0.5) * GRID_SIZE && (*agentPtr).position.x < (x + 1.5) * GRID_SIZE)
                {
                    //Top
                    if ((*agentPtr).position.y > (y - 0.5) * GRID_SIZE && (*agentPtr).position.y < y * GRID_SIZE)
                    {
                        (*agentPtr).position.y = (y - 0.5) * GRID_SIZE;
                    }
                    else
                    //Bottom
                    if ((*agentPtr).position.y < (y + 1.5) * GRID_SIZE && (*agentPtr).position.y > (y + 1) * GRID_SIZE)
                    {
                        (*agentPtr).position.y = (y + 1.5) * GRID_SIZE;
                    }
                }
            }
        }
    }

    //Horizontal
    (*agentPtr).position.x += movement.x;
    for (int y = 0; y < SIZE_Y; y++)
    {
        for (int x = 0; x < SIZE_X; x++)
        {
            if (map[y * SIZE_X + x].solid)
            {
                if ((*agentPtr).position.y > (y - 0.5) * GRID_SIZE && (*agentPtr).position.y < (y + 1.5) * GRID_SIZE)
                {
                    //Right
                    if ((*agentPtr).position.x > (x - 0.5) * GRID_SIZE && (*agentPtr).position.x < x * GRID_SIZE)
                    {
                        (*agentPtr).position.x = (x - 0.5) * GRID_SIZE;
                    }
                    else
                    //Left
                    if ((*agentPtr).position.x < (x + 1.5) * GRID_SIZE && (*agentPtr).position.x > (x + 1) * GRID_SIZE)
                    {
                        (*agentPtr).position.x = (x + 1.5) * GRID_SIZE;
                    }
                }
            }
        }    
    }
}

void DrawHealthBar(Vector2 position, int health, int maxHealth)
{
    int width = 50;
    float fill = (float)health / maxHealth;
    int padding = 2;

    Color c = (Color){Clamp(Remap(fill, 1, 0.5, 0, 256), 0, 255), Clamp(Remap(fill, 0, 0.5, 0, 256), 0, 255), 0, 255};
    
    DrawRectangle(position.x - width * 0.5, position.y - 30, width, 10, BLACK);
    DrawRectangle(position.x - width * 0.5 + padding, position.y - 30 + padding, (width - padding * 2) * fill, 10 - padding * 2, c);
}

Vector4 DirectionToStrangeVector(Direction dir)
{
    switch (dir)
    {
    case UP:
        return (Vector4){-1, -1, 1, -1};
    case RIGHT:
        return (Vector4){-1, -1, -1, 1};
    case DOWN:
        return (Vector4){1, 1, -1, 1};
    case LEFT:
        return (Vector4){1, 1, 1, -1};
    default:
        return (Vector4){0, 0, 0, 0};
    }
}

bool IsInDangerZone(Vector2 testPos, Agent originAgent, Attack attack)
{
    float dist = Vector2Distance(testPos, originAgent.position);
    Vector2 diff = Vector2Subtract(originAgent.position, testPos);
    Vector4 vec = DirectionToStrangeVector(originAgent.direction);
    if (
        attack.active &&
        dist < attack.radius &&
        diff.x * vec.x > diff.y * vec.y &&
        diff.x * vec.z > diff.y * vec.w
    ) return true;
    
    return false;
}

void DrawAgent(Agent agent)
{
    DrawTexturePro( 
        agent.sprite,
        (Rectangle){agent.direction * 16, 0, 16, 16},
        (Rectangle){agent.position.x - GRID_SIZE * 0.5, agent.position.y - GRID_SIZE * 0.5, GRID_SIZE, GRID_SIZE},
        Vector2Zero(),
        0,
        WHITE
    );

    DrawRectangle(agent.position.x - GRID_SIZE * 0.5, agent.position.y - GRID_SIZE * 0.5, GRID_SIZE, GRID_SIZE, (Color){255, 255, 255, (agent.invincible / 0.2) * 255});

    for (float y = -agent.attack.radius; y < agent.attack.radius; y++)
    {
        for (float x = -agent.attack.radius; x < agent.attack.radius; x++)
        {
            Vector2 vector = Vector2Add((Vector2){x, y}, agent.position);

            if (IsInDangerZone(vector, agent, agent.attack))
            {
                DrawPixelV(vector, (Color){255, 255, 255, 128});
            }
        }
    }

    DrawHealthBar(agent.position, agent.health, agent.maxHealth);
}

bool EnemiesDead(Agent agents[])
{
    for (int i = 0; i < MAX_AGENTS; i++)
    {
        if (agents[i].health > 0) return false;
    }
    
    return true;
}

void DrawTextCentered(const char *str, int yPos, int size, Color color)
{
    DrawText(str, SIZE_X * GRID_SIZE / 2 - MeasureText(str, size) / 2, yPos, size, color);
}

float GetBobPos(BobStat bs)
{
    return bs.origin + sin(GetTime() * bs.speed) * bs.variance;
}