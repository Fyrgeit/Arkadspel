#include "functions.c"

int main()
{
    GameState gameState = MENU;
    
    const int screenWidth = SIZE_X * GRID_SIZE;
    const int screenHeight = SIZE_Y * GRID_SIZE;

    Tile map[SIZE_X * SIZE_Y];

    for (int y = 0; y < SIZE_Y; y++)
    {
        for (int x = 0; x < SIZE_X; x++)
        {
            map[y * SIZE_X + x] = (Tile){0, false};

            if (x == 1)
            {
                map[y * SIZE_X + x] = (Tile){1, true};
                if (y > 8 && y < SIZE_Y - 9)
                {
                    map[y * SIZE_X + x] = (Tile){0, false};
                }
            }

            if (x == 0)
            {
                map[y * SIZE_X + x] = (Tile){3, true};
                if (y > 8 && y < SIZE_Y - 9)
                {
                    map[y * SIZE_X + x] = (Tile){1, true};
                }
            }

            if (x == SIZE_X - 1)
            {
                map[y * SIZE_X + x] = (Tile){2, true};
            }

            if (y == 0 || y == SIZE_Y - 1)
            {
                map[y * SIZE_X + x] = (Tile){3, true};
            }
        }
    }

    float gameTimer = 5;
    float foodTimer = 2;
    bool incoming = false;

    InitWindow(screenWidth, screenHeight, "Arkadspel 2");

    Texture2D spriteSheet = LoadTexture("sprite-sheet.png");
    Texture2D foodTexture = LoadTexture("star.png");
    Texture2D enemyEasyTexture = LoadTexture("enemy-easy.png");
    Texture2D enemyHardTexture = LoadTexture("enemy-hard.png");
    Texture2D playerTexture = LoadTexture("player.png");
    Texture2D buttonTexture = LoadTexture("button.png");

    Vector2 playerSpawn = (Vector2){800, screenHeight / 2};

    Agent player = (Agent){PLAYER, playerSpawn, 100, 100, 180, 0, (Attack){GRID_SIZE * 4, false, 0, 0.2, 0.1}, DOWN, playerTexture};
    
    Agent agents[MAX_AGENTS];

    for (int i = 0; i < MAX_AGENTS; i++)
    {
        agents[i].health = -1;
    }

    Food foods[MAX_FOODS];

    Agent enemies[] = {
        (Agent){ENEMY, Vector2Zero(), 30, 30, 60, 0, (Attack){GRID_SIZE * 2, false, 0, 1, 0.1}, DOWN, enemyEasyTexture},
        (Agent){ENEMY, Vector2Zero(), 50, 50, 100, 0, (Attack){GRID_SIZE * 3, false, 0, 0.5, 0.1}, DOWN, enemyHardTexture}
    };

    Wav waves[] = {
        (Wav){0, 1},
        (Wav){0, 2},
        (Wav){0, 3},
        (Wav){1, 1},
        (Wav){1, 2},
        (Wav){0, 6},
        (Wav){1, 4},
        (Wav){1, 5},
        (Wav){0, 12},
        (Wav){1, 8},
        (Wav){0, 16},
        (Wav){1, 16},
    };

    int waveIndex = 0;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            switch (gameState)
            {
            case PLAY:
                gameState = PAUSED;
                break;
            case PAUSED:
                gameState = PLAY;
                break;
            
            default:
                break;
            }
        }
        
        switch (gameState)
        {
            case PLAY:
            {
                incoming = false;

                if (gameTimer <= 0)
                {
                    for (int i = 0; i < waves[waveIndex].count; i++)
                    {
                        AddAgent(agents, enemies[waves[waveIndex].enemyIndex], (Vector2){50, screenHeight / 2});
                    }
                    
                    waveIndex++;
                    gameTimer = 5;
                }
                else if (EnemiesDead(agents))
                {
                    gameTimer -= GetFrameTime();
                    incoming = true;
                }

                if (foodTimer <= 0)
                {
                    AddFood(foods, (Food){(Vector2){
                        drand48() * (screenWidth - 100) + 50, 
                        drand48() * (screenHeight - 100) + 50
                    }, true});
                    foodTimer = 2;
                }
                else
                {
                    foodTimer -= GetFrameTime();
                }

                for (int i = 0; i < MAX_FOODS; i++)
                {
                    if (!foods[i].alive) continue;
                    if (Vector2Distance(foods[i].position, player.position) > GRID_SIZE) continue;
                    
                    player.health = Clamp(player.health + 10, 0, 100);
                    foods[i].alive = false;
                }
                
                //player================================
                Vector2 playerMovement = Vector2Zero();
                
                if (IsKeyDown(KEY_D)) playerMovement.x++;
                if (IsKeyDown(KEY_A)) playerMovement.x--;
                if (IsKeyDown(KEY_S)) playerMovement.y++;
                if (IsKeyDown(KEY_W)) playerMovement.y--;

                playerMovement = Vector2Normalize(playerMovement);

                if (Vector2Length(playerMovement) > 0.01 && !IsKeyDown(KEY_SPACE))
                {
                    float dir = atan2f(playerMovement.y, playerMovement.x);

                    if (dir > -1 && dir < 1) player.direction = RIGHT;
                    if (dir > 1 && dir < PI - 1) player.direction = DOWN;
                    if (dir > PI - 1 || dir < -PI + 1) player.direction = LEFT;
                    if (dir > -PI + 1 && dir < -1) player.direction = UP;
                }

                playerMovement = Vector2Scale(playerMovement, player.speed * GetFrameTime());
                
                AgentCollision(&player, playerMovement, map);

                //Attack
                if (player.attack.timer <= 0)
                {
                    if (IsKeyPressed(KEY_K))
                    {
                        player.attack.timer = player.attack.cooldown;
                        player.attack.active = true;
                    }
                }
                else
                {
                    player.attack.timer -= GetFrameTime();
                    if (player.attack.timer < player.attack.cooldown - player.attack.dragTime)
                    {
                        player.attack.active = false;
                    }
                }

                //Damage
                for (int i = 0; i < MAX_AGENTS; i++)
                {
                    if (agents[i].health <= 0) continue;

                    if (IsInDangerZone(player.position, agents[i], agents[i].attack) && player.invincible <= 0)
                    {
                        player.health -= 10;
                        player.invincible = 0.2;
                        break;
                    }
                }
                
                if (player.invincible > 0)
                {
                    player.invincible -= GetFrameTime();
                }

                if (player.health <= 0) gameState = GAME_OVER;

                //Enemies================================
                for (int i = 0; i < MAX_AGENTS; i++)
                {
                    if (agents[i].health <= 0) continue;
                    
                    Agent *agentPtr = &agents[i];
                    Vector2 agentMovement = Vector2Zero();
                    
                    //Player attraction
                    agentMovement = Vector2Normalize(Vector2Subtract(player.position, (*agentPtr).position));

                    //Player repellant
                    float weight = Clamp(Remap(Vector2Distance(player.position, (*agentPtr).position), (*agentPtr).attack.radius * 1.2, (*agentPtr).attack.radius * 0.8, 0, 1), 0, 2);
                    agentMovement = Vector2Add(agentMovement, Vector2Scale(Vector2Normalize(Vector2Subtract((*agentPtr).position, player.position)), weight));
                    
                    //Enemy repellant
                    for (int n = 0; n < MAX_AGENTS; n++)
                    {
                        if (agents[n].health <= 0) continue;

                        float weight = Clamp(Remap(Vector2Distance((*agentPtr).position, agents[n].position), GRID_SIZE * 1.5, GRID_SIZE, 0, 1), 0, 1);
                        agentMovement = Vector2Add(agentMovement, Vector2Scale(Vector2Normalize(Vector2Subtract((*agentPtr).position, agents[n].position)), weight));
                    }

                    if (Vector2Length(agentMovement) > 1)
                    {
                        agentMovement = Vector2Normalize(agentMovement);
                    }
                    
                    //Direction
                    Vector2 diff = Vector2Subtract(player.position, (*agentPtr).position);
                    float dir = atan2f(diff.y, diff.x);

                    if (dir > PI * -0.25 && dir < PI * 0.25) (*agentPtr).direction = RIGHT;
                    if (dir > PI * 0.25 && dir < PI * 0.75) (*agentPtr).direction = DOWN;
                    if (dir > PI * 0.75 || dir < PI * -0.75) (*agentPtr).direction = LEFT;
                    if (dir > PI * -0.75 && dir < PI * -0.25) (*agentPtr).direction = UP;
                    
                    agentMovement = Vector2Scale(agentMovement, (*agentPtr).speed * GetFrameTime());

                    AgentCollision(agentPtr, agentMovement, map);

                    //Attack
                    if ((*agentPtr).attack.timer <= 0)
                    {
                        if (Vector2Distance((*agentPtr).position, player.position) < (*agentPtr).attack.radius * 1.5)
                        {
                            (*agentPtr).attack.timer = (*agentPtr).attack.cooldown;
                            (*agentPtr).attack.active = true;
                        }
                    }
                    else
                    {
                        (*agentPtr).attack.timer -= GetFrameTime();
                        if ((*agentPtr).attack.timer < (*agentPtr).attack.cooldown - (*agentPtr).attack.dragTime)
                        {
                            (*agentPtr).attack.active = false;
                        }
                    }

                    //Damage
                    if (IsInDangerZone((*agentPtr).position, player, player.attack) && (*agentPtr).invincible <= 0)
                    {
                        (*agentPtr).health -= 10;
                        (*agentPtr).invincible = 0.2;
                    }
                    
                    if ((*agentPtr).invincible > 0)
                    {
                        (*agentPtr).invincible -= GetFrameTime();
                    }
                }
            } break;
        
            default: break;
        }
        
        BeginDrawing();
        ClearBackground(BLACK);

        switch (gameState)
        {
            case PLAY:
            {
                //Draw map
                for (int y = 0; y < SIZE_Y; y++)
                {
                    for (int x = 0; x < SIZE_X; x++)
                    {

                        DrawTexturePro(
                            spriteSheet,
                            (Rectangle){map[y * SIZE_X + x].tileId * 16, 0, 16, 16},
                            (Rectangle){x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE},
                            Vector2Zero(), 0, WHITE
                        );
                    }    
                }

                //Draw foods
                for (int i = 0; i < MAX_FOODS; i++)
                {
                    if (!foods[i].alive) continue;

                    DrawTexturePro(
                        foodTexture,
                        (Rectangle){0, 0, 8, 8},
                        (Rectangle){foods[i].position.x - 8, foods[i].position.y - 8, 16, 16},
                        Vector2Zero(),
                        0,
                        WHITE
                    );
                }
                
                //Draw agents
                for (int i = 0; i < MAX_AGENTS; i++)
                {
                    if (agents[i].health > 0)
                    {
                        DrawAgent(agents[i]);
                    }
                }

                DrawAgent(player);

                if (incoming) DrawTextCentered("Enemies incoming!", 0, 30, WHITE);

                Rectangle buttonRect = (Rectangle){screenWidth - GRID_SIZE, 0, GRID_SIZE, GRID_SIZE};

                if (CheckCollisionPointRec(GetMousePosition(), buttonRect))
                {
                    DrawTexturePro(buttonTexture, (Rectangle){16, 0, 16, 16}, buttonRect, Vector2Zero(), 0, WHITE);

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        gameState = PAUSED;
                    }
                }
                else
                {
                    DrawTexturePro(buttonTexture, (Rectangle){0, 0, 16, 16}, buttonRect, Vector2Zero(), 0, WHITE);
                }
            } break;

            case PAUSED:
            {
                //Draw map
                for (int y = 0; y < SIZE_Y; y++)
                {
                    for (int x = 0; x < SIZE_X; x++)
                    {

                        DrawTexturePro(
                            spriteSheet,
                            (Rectangle){map[y * SIZE_X + x].tileId * 16, 0, 16, 16},
                            (Rectangle){x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE},
                            Vector2Zero(), 0, WHITE
                        );
                    }    
                }

                //Draw foods
                for (int i = 0; i < MAX_FOODS; i++)
                {
                    if (!foods[i].alive) continue;

                    DrawTexturePro(
                        foodTexture,
                        (Rectangle){0, 0, 8, 8},
                        (Rectangle){foods[i].position.x - 8, foods[i].position.y - 8, 16, 16},
                        Vector2Zero(),
                        0,
                        WHITE
                    );
                }
                
                //Draw agents
                for (int i = 0; i < MAX_AGENTS; i++)
                {
                    if (agents[i].health > 0)
                    {
                        DrawAgent(agents[i]);
                    }
                }

                DrawAgent(player);

                if (incoming) DrawTextCentered("Enemies incoming!", 0, 30, WHITE);
                
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 64});

                DrawTextCentered("Game paused", 100, 80, WHITE);
                DrawTextCentered("Controls", 200, 60, WHITE);
                DrawTextCentered("WASD - Move\nSpace - Freeze rotation\nK - Fight\nEnter - Pause", 280, 40, WHITE);

                Rectangle buttonRect = (Rectangle){screenWidth - GRID_SIZE, 0, GRID_SIZE, GRID_SIZE};

                if (CheckCollisionPointRec(GetMousePosition(), buttonRect))
                {
                    DrawTexturePro(buttonTexture, (Rectangle){48, 0, 16, 16}, buttonRect, Vector2Zero(), 0, WHITE);

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        gameState = PLAY;
                    }
                }
                else
                {
                    DrawTexturePro(buttonTexture, (Rectangle){32, 0, 16, 16}, buttonRect, Vector2Zero(), 0, WHITE);
                }
            } break;

            case MENU:
            {
                ClearBackground(DARKGREEN);
                DrawTextCentered("KILL THE", 20, 100, WHITE);
                DrawTextCentered("ANGRY PEOPLE", 120, 100, WHITE);

                BobStat bobStats[] = {
                    (BobStat){500, 10, 2},
                    (BobStat){400, 5, 4},
                    (BobStat){640, 5, 5},
                    (BobStat){550, 5, 6}
                };

                DrawTexturePro(
                    playerTexture,
                    (Rectangle){32, 0, 16, 16},
                    (Rectangle){800, GetBobPos(bobStats[0]), 300, 300},
                    (Vector2){150, 150},
                    -20,
                    WHITE
                );
                DrawTexturePro(
                    enemyEasyTexture,
                    (Rectangle){48, 0, 16, 16},
                    (Rectangle){200, GetBobPos(bobStats[1]), 200, 200},
                    (Vector2){100, 100},
                    10,
                    WHITE
                );
                DrawTexturePro(
                    enemyEasyTexture,
                    (Rectangle){48, 0, 16, 16},
                    (Rectangle){150, GetBobPos(bobStats[2]), 200, 200},
                    (Vector2){100, 100},
                    -10,
                    WHITE
                );
                DrawTexturePro(
                    enemyHardTexture,
                    (Rectangle){48, 0, 16, 16},
                    (Rectangle){450, GetBobPos(bobStats[3]), 200, 200},
                    (Vector2){100, 100},
                    -5,
                    WHITE
                );

                Rectangle buttonRect = (Rectangle){screenWidth / 2 - 150, 250, 300, 100};

                if (CheckCollisionPointRec(GetMousePosition(), buttonRect))
                {
                    DrawRectangleRec(buttonRect, WHITE);
                    DrawTextCentered("PLAY", 260, 80, BLACK);

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        gameState = PLAY;
                    }
                }
                else
                {
                    DrawRectangleRec(buttonRect, GREEN);
                    DrawTextCentered("PLAY", 260, 80, WHITE);
                }
            } break;

            case GAME_OVER:
            {
                DrawTextCentered("GAME OVER", 220, 80, WHITE);
                DrawTextCentered(TextFormat("You made it past wave %i", waveIndex - 1), 300, 40, WHITE);

                Rectangle buttonRect = (Rectangle){screenWidth / 2 - 200, 400, 400, 50};

                if (CheckCollisionPointRec(GetMousePosition(), buttonRect))
                {
                    DrawRectangleRec(buttonRect, WHITE);
                    DrawTextCentered("RETURN TO MENU", 405, 40, BLACK);

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        gameState = MENU;
                        player.health = 100;
                        player.position = playerSpawn;
                        waveIndex = 0;
                        gameTimer = 5;
                        foodTimer = 5;

                        for (int i = 0; i < MAX_AGENTS; i++)
                        {
                            agents[i].health = -1;
                        }
                        
                        for (int i = 0; i < MAX_FOODS; i++)
                        {
                            foods[i].alive = false;
                        }
                    }
                }
                else
                {
                    DrawRectangleRec(buttonRect, DARKGRAY);
                    DrawTextCentered("RETURN TO MENU", 405, 40, WHITE);
                }
            } break;
        
            default: break;
        }
        
        //Debug
        DrawText(TextFormat("FPS: %i", GetFPS()), 5, 5, 20, WHITE);
        EndDrawing();
    }
    
    return 0;
}