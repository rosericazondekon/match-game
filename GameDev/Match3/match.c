#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>
#include "math.h"
#include "time.h"

// Game constants
#define BOARD_SIZE 12
#define TILE_SIZE 72
#define TILE_TYPES 5
#define SCORE_FONT_SIZE 44
#define MATCHED_DISPLAY_TIME 45  // frames to show matched tiles in green
#define MAX_SCORE_POPUPS 32

Music background_music;
Sound match_sound;

// Tile characters for display
// const char *tile_chars[TILE_TYPES] = {
//     "\u25C7", // ◇ diamond
//     "\u25A1", // □ square
//     "\u25B3", // △ triangle
//     "\u25CB", // ○ circle
//     "\u2B21", // ⬡ hexagon
// };
const char tile_chars[TILE_TYPES] = {'@', '#', '$', '%', '&'};

// Game board
char board[BOARD_SIZE][BOARD_SIZE];
char matched[BOARD_SIZE][BOARD_SIZE] = {0}; // To track matched tiles
float fall_offset[BOARD_SIZE][BOARD_SIZE] = {0}; // To animate falling tiles

int score = 0; // Initial score
Vector2 grid_to_origin;
Texture2D background;
Font score_font;
Vector2 selected_tile = {-1, -1}; // No tile selected initially
float fall_speed = 8.0f; // Speed at which tiles fall

float score_scale = 1.0f; // Scale for score popups
float score_scale_velocity = 0.0f; // Velocity for score popup scaling
bool score_animating = false; // Flag to indicate if score popup is animating

typedef enum {
    STATE_IDLE,
    STATE_SHOWING_MATCHES,
    STATE_ANIMATING
} TileState;

TileState tile_states;
int matched_display_timer = 0;

typedef struct {
    Vector2 position;
    int amount;
    float lifetime;
    float alpha;
    bool active;
} ScorePopup;

ScorePopup score_popups[MAX_SCORE_POPUPS] = {0};

void add_score_popup(int x, int y, int amount, Vector2 grid_origin);

// Function to generate a random tile character
const char random_tile() {
    return tile_chars[rand() % TILE_TYPES];
}

// Function to find matches in the board and mark them
bool find_matches() {
    bool found = false;
    for(int j = 0; j < BOARD_SIZE; j++){
        for(int i = 0; i < BOARD_SIZE; i++){
            matched[j][i] = false;
        }
    }

    for(int j = 0; j < BOARD_SIZE; j++){
        for(int i = 0; i < BOARD_SIZE - 2; i++){
            char t = board[j][i];
            if(t == board[j][i + 1] && t == board[j][i + 2]){
                matched[j][i] = matched[j][i + 1] = matched[j][i + 2] = true;
                score += 10;
                found = true;
                PlaySound(match_sound); // Play match sound

                score_animating = true; // Start score popup animation
                score_scale = 2.0f; // Initial scale for popup
                score_scale_velocity = -2.5f; // Scale will shrink over time

                add_score_popup(i, j, 10, grid_to_origin);
            }
        }
    }

    for(int j = 0; j < BOARD_SIZE - 2; j++){
        for(int i = 0; i < BOARD_SIZE; i++){
            char t = board[j][i];
            if(t == board[j + 1][i] && t == board[j + 2][i]){
                matched[j][i] = matched[j + 1][i] = matched[j + 2][i] = true;
                score += 10;
                found = true;
                PlaySound(match_sound); // Play match sound

                score_animating = true; // Start score popup animation
                score_scale = 2.0f; // Initial scale for popup
                score_scale_velocity = -2.5f; // Scale will shrink over time

                add_score_popup(i, j, 10, grid_to_origin);
            }
        }
    }
    return found;
}

// Function to resolve matches by making tiles fall and filling empty spaces
void resolve_matches(){
    for(int i = 0; i < BOARD_SIZE; i++){
        int write_j = BOARD_SIZE - 1;
        for(int j = BOARD_SIZE - 1; j >= 0; j--){
            if(!matched[j][i]){
                if(j != write_j){
                    board[write_j][i] = board[j][i];
                    fall_offset[write_j][i] = (write_j - j) * TILE_SIZE;
                    board[j][i] = ' ';
                }
                write_j--;
            }
        }

        while(write_j >= 0){
            board[write_j][i] = random_tile();
            fall_offset[write_j][i] = (write_j + 1) * TILE_SIZE;
            write_j--;
        }
    }

    // Clear matched array so new tiles don't stay green
    for(int j = 0; j < BOARD_SIZE; j++){
        for(int i = 0; i < BOARD_SIZE; i++){
            matched[j][i] = false;
        }
    }

    tile_states = STATE_ANIMATING;
}

void swap_tiles(int x1, int y1, int x2, int y2){
    char temp = board[y1][x1];
    board[y1][x1] = board[y2][x2];
    board[y2][x2] = temp;
}

bool are_tiles_adjacent(Vector2 a, Vector2 b){
    return (abs((int)a.x - (int)b.x) + abs((int)a.y - (int)b.y)) == 1;
}

void add_score_popup(int x, int y, int amount, Vector2 grid_origin){
    for(int i = 0; i < MAX_SCORE_POPUPS; i++){
        if(!score_popups[i].active){
            score_popups[i].position = (Vector2){
                grid_origin.x + x * TILE_SIZE + TILE_SIZE / 2,
                grid_origin.y + y * TILE_SIZE + TILE_SIZE / 2
            };
            score_popups[i].amount = amount;
            score_popups[i].lifetime = 1.0f; 
            score_popups[i].alpha = 1.0f;
            score_popups[i].active = true;
            break;
        }
    }
}

// Function to initialize the game board with random tiles
void init_board(){
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = random_tile();
        }
    }

    int grid_width = BOARD_SIZE * TILE_SIZE;
    int grid_height = BOARD_SIZE * TILE_SIZE;

    grid_to_origin = (Vector2){
        (GetScreenWidth() - grid_width) / 2, // Center the grid horizontally
        (GetScreenHeight() - grid_height) / 2
    }; 

    if(find_matches()){
        resolve_matches(); // Ensure no initial matches on the board
    } else{
        tile_states = STATE_IDLE; // Set initial state to idle
    }
}


int main(){
    const int screen_width = 1600;
    const int screen_height = 914;

    InitWindow(screen_width, screen_height, "Match-3 Game");
    SetTargetFPS(60);

    srand(time(NULL)); // Seed random number generator

    InitAudioDevice(); // Initialize audio device

    background = LoadTexture("assets/bg3.jpg"); // Load background texture
    score_font = LoadFontEx("assets/04b03.ttf", SCORE_FONT_SIZE, NULL, 0); // Load custom font for score display
    background_music = LoadMusicStream("assets/music2.mp3"); // Load background music
    match_sound = LoadSound("assets/match.mp3"); // Load match sound

    PlayMusicStream(background_music); // Start playing background music
    SetMusicVolume(background_music, 0.4f); // Set background music volume
    
    init_board(); // Initialize the game board
    Vector2 mouse = {0, 0};

    while(!WindowShouldClose()){
        UpdateMusicStream(background_music);

        // Update game logic
        mouse = GetMousePosition();
        if(tile_states == STATE_IDLE && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            int col = (mouse.x - grid_to_origin.x) / TILE_SIZE;
            int row = (mouse.y - grid_to_origin.y) / TILE_SIZE;

            if (col >= 0 && col < BOARD_SIZE && row >= 0 && row < BOARD_SIZE) {
                Vector2 current_tile = (Vector2){col, row};
                if(selected_tile.x < 0){
                    selected_tile = current_tile; // Select the tile
                } else {
                    if(are_tiles_adjacent(selected_tile, current_tile)){
                        swap_tiles(selected_tile.x, selected_tile.y, current_tile.x, current_tile.y);
                        if(find_matches()){
                            tile_states = STATE_SHOWING_MATCHES;
                            matched_display_timer = MATCHED_DISPLAY_TIME;
                        } else {
                            swap_tiles(selected_tile.x, selected_tile.y, current_tile.x, current_tile.y); // Swap back if no match
                        }
                    }
                    selected_tile = (Vector2){-1, -1}; // Deselect after attempting swap
                }
            }
        }

        if(tile_states == STATE_SHOWING_MATCHES){
            matched_display_timer--;
            if(matched_display_timer <= 0){
                resolve_matches();
            }
        }

        if(tile_states == STATE_ANIMATING){
            bool still_animating = false;

            for(int j = 0; j < BOARD_SIZE; j++){
                for(int i = 0; i < BOARD_SIZE; i++){
                    if(fall_offset[j][i] > 0){
                        fall_offset[j][i] -= fall_speed;
                        if(fall_offset[j][i] < 0){
                            fall_offset[j][i] = 0; // Clamp to zero
                        } else{
                            still_animating = true; // Continue animating if any tile is still falling
                        }
                    }
                }
            }
            if(!still_animating){
                // Animation complete - check for new matches
                if(find_matches()){
                    tile_states = STATE_SHOWING_MATCHES;
                    matched_display_timer = MATCHED_DISPLAY_TIME;
                } else {
                    tile_states = STATE_IDLE;
                }
            }
        }

        // Update score popups
        for(int i = 0; i < MAX_SCORE_POPUPS; i++){
            if(score_popups[i].active){
                score_popups[i].lifetime -= GetFrameTime(); // Assuming 60 FPS
                score_popups[i].position.y -= 30 * GetFrameTime(); // Move up over time
                score_popups[i].alpha = score_popups[i].lifetime; // Fade out over time
                if(score_popups[i].lifetime <= 0){
                    score_popups[i].active = false; // Deactivate when lifetime is over
                }
            }
        }

        // Update score popup animation
        if(score_animating){
            score_scale += score_scale_velocity * GetFrameTime(); // Scale animation
            if(score_scale <= 1.0f){
                score_scale = 1.0f;
                score_animating = false; // End animation when scale returns to normal
            }
        }
        

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw the background
        DrawTexturePro(
            background,
            (Rectangle){
                0, 0, background.width, background.height
            },
            (Rectangle){
                0, 0, GetScreenWidth(), GetScreenHeight()
            },
            (Vector2){0, 0},
            0.0f,
            WHITE
        );

        DrawRectangle(
            grid_to_origin.x - 4, 
            grid_to_origin.y - 4, 
            BOARD_SIZE * TILE_SIZE + 8, 
            BOARD_SIZE * TILE_SIZE + 8,
            Fade(DARKGRAY, 0.6f)
        ); // Draw border around the grid

        // Draw the game board
        for (int j = 0; j < BOARD_SIZE; j++) {
            for (int i = 0; i < BOARD_SIZE; i++) {
                Rectangle rect = {
                    grid_to_origin.x + i * TILE_SIZE, // x position
                    grid_to_origin.y + j * TILE_SIZE, // y position
                    TILE_SIZE,     // width
                    TILE_SIZE      // height
                };
                DrawRectangleLinesEx(rect, 1, DARKGRAY); // Draw tile background

                if(board[j][i] != ' '){
                    DrawTextEx(
                        GetFontDefault(), 
                        TextFormat("%c", board[j][i]),
                        (Vector2) {
                            rect.x + 24, 
                            rect.y + 16 - fall_offset[j][i]
                        }, 
                        38.0f, 
                        0.0f,
                        matched[j][i] ? GREEN : WHITE
                    );
                }
            }
        }

        // Draw selected tile
        if(selected_tile.x >= 0){
            Rectangle selected_rect = {
                grid_to_origin.x + selected_tile.x * TILE_SIZE, 
                grid_to_origin.y + selected_tile.y * TILE_SIZE, 
                TILE_SIZE, 
                TILE_SIZE
            };
            DrawRectangleLinesEx(selected_rect, 4, YELLOW); // Highlight selected tile
        }

        // Draw the score
        DrawTextEx(
            score_font, 
            TextFormat("SCORE: %d", score), 
            (Vector2){20, 20}, 
            SCORE_FONT_SIZE * score_scale, 
            1.0f,
            YELLOW
        );

        // Draw score popups
        for(int i = 0; i < MAX_SCORE_POPUPS; i++){
            if(score_popups[i].active){
                DrawText(
                    TextFormat("+%d", score_popups[i].amount),
                    score_popups[i].position.x,
                    score_popups[i].position.y,
                    SCORE_FONT_SIZE/2,
                    Fade(YELLOW, score_popups[i].alpha)
                );
            }
        }

        EndDrawing();
    }

    StopMusicStream(background_music); // Stop background music
    UnloadTexture(background); // Unload background texture
    UnloadFont(score_font); // Unload font
    UnloadMusicStream(background_music); // Unload background music
    UnloadSound(match_sound); // Unload match sound

    CloseAudioDevice(); // Close audio device

    CloseWindow(); // Close the window and clean up resources
    
    return 0;
}