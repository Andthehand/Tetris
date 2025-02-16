#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphics.h"
#include "game.h"

//-----------------------------------------------------------------------------
// Define symbolic constants used by the program
//-----------------------------------------------------------------------------
#define TETRIS_HEIGHT                                                        20
#define TETRIS_WIDTH                                                         10
#define TETRIS_NO_BLOCK                                                       0
#define TETRIS_LOOPS_TILL_FALL                                               15
#define TETRIS_FILL_DELAY                                                    50
#define TETRIS_LOOP_DELAY                                                    16

//-----------------------------------------------------------------------------
// Define a structure to hold different data types used by the program
//-----------------------------------------------------------------------------
typedef struct {
  int8_t x;
  int8_t y;
} Vector2;

typedef enum {
  BLANK = 0,
  RED = 1,
} BLOCK_COLOR_INDEX;

typedef struct {
  int8_t blocks[4][4];
  BLOCK_COLOR_INDEX color;
  uint8_t size;
} Shape;

typedef enum {
  NONE = 0,
  UPDATE_GRID,
  UPDATE_SELECTED_BLOCK
} CheckType;

//-----------------------------------------------------------------------------
// Define function prototypes used by the program
//-----------------------------------------------------------------------------
void input_handler();
void clear_rows();
Shape rotate_shape();
void place_shape();
bool can_move(int8_t x_step, int8_t y_step);
bool move(int8_t x_step, int8_t y_step);
void render_game_over();
void render_screen();

//-----------------------------------------------------------------------------
// Define global variables and structures here.
// NOTE: when possible avoid using global variables
//-----------------------------------------------------------------------------
uint8_t block_grid[TETRIS_WIDTH][TETRIS_HEIGHT] = {{0}};

const Shape SHAPES[] = {
  {{ {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, RED, 4}, // I
  {{ {1, 1}, {1, 1} }, RED, 2},                                         // O
  {{ {0, 1, 1}, {1, 1, 0}, {0, 0, 0} }, RED, 3},                        // S
  {{ {1, 1, 0}, {0, 1, 1}, {0, 0, 0} }, RED, 3},                        // Z
  {{ {1, 0, 0}, {1, 1, 1}, {0, 0, 0} }, RED, 3},                        // L
  {{ {0, 0, 1}, {1, 1, 1}, {0, 0, 0} }, RED, 3},                        // J
  {{ {0, 1, 0}, {1, 1, 1}, {0, 0, 0} }, RED, 3}                         // T
};

const uint8_t BLOCK_COLOR[][3] = {
    { 0x00, 0x00, 0x00 }, // Black
    { 0xFF, 0x00, 0x00 }, // Red
};

Vector2 selected_block_pos = {0};
Shape current_block;
bool game_over = false;

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  Initializes the game state by selecting the initial block and positioning 
//  it at the starting location.
//
// INPUT PARAMETERS:
//  none
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  none
//-----------------------------------------------------------------------------
void init_game(void)
{
  current_block = SHAPES[rand() % (sizeof(SHAPES) / sizeof(SHAPES[0]))];
  selected_block_pos.x = (TETRIS_WIDTH - current_block.size) / 2;
  selected_block_pos.y = 0;
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  The main game loop responsible for rendering the screen, handling input,
//  updating game logic, and managing the timing of block movement and placement.
//
// INPUT PARAMETERS:
//  none
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  none
// 
//-----------------------------------------------------------------------------
void game_loop(void)
{
  uint32_t loop_count = 0;
  while (!game_over) {
    render_screen();
    input_handler();

    loop_count++;
    if((loop_count % TETRIS_LOOPS_TILL_FALL) == 0)
    {
      loop_count = 0;
      
      bool moved = move(0, 1);
      if(!moved)
      {
        place_shape();
      }
    }

    sleep_ms(TETRIS_LOOP_DELAY);
  }

  render_game_over();
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  Handles user input from the joystick and push buttons to move, rotate, 
//  or accelerate the current block in the Tetris game. Ensures movements 
//  are valid and updates the game state accordingly.
//
// INPUT PARAMETERS:
//  none
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  none
// 
//-----------------------------------------------------------------------------
void input_handler() {
  char input = stdio_getchar_timeout_us(0);

  switch(input) {
    case 'a':
      if (can_move(1, 0)) {
        selected_block_pos.x++;
      }
      break;
    case 's':
      if (can_move(0, 1)) {
        selected_block_pos.y++;
      }
      break;
    case 'd':
      if (can_move(-1, 0)) {
        selected_block_pos.x--;
      }
      break;
    case ' ':
      Shape original_shape = current_block;
      current_block = rotate_shape();

      if (!can_move(0, 0)) {
        current_block = original_shape;
      }
      break;
  }
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  Checks each row of the Tetris grid to determine if it is fully occupied.
//  If a full row is found, it clears the row, shifts rows above it downward, 
//  and updates the game state to reflect the changes.
//
// INPUT PARAMETERS:
//  none
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  none
// 
//-----------------------------------------------------------------------------
void clear_rows() {
  for (int y = 0; y < TETRIS_HEIGHT; y++) {
    bool full_row = true;
    for (int x = 0; x < TETRIS_WIDTH; x++) {
      if (!block_grid[x][y]) {
        full_row = false;
      }
    }
    
    if (full_row) {
      for (int ny = y; ny > 0; ny--) {
        for (int x = 0; x < TETRIS_WIDTH; x++) {
          block_grid[x][ny] = block_grid[x][ny - 1];
        }
      }

      memset(block_grid[0], 0, TETRIS_WIDTH);
    }
  }
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  Rotates the current Tetris block 90 degrees clockwise. If the rotated block 
//  cannot be placed in its current position due to collisions, the rotation 
//  is canceled, and the block remains in its original orientation.
//
// INPUT PARAMETERS:
//  none
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  Shape: The rotated block if the rotation is valid; otherwise, 
//         the original block.
// 
//-----------------------------------------------------------------------------
Shape rotate_shape() {
  Shape rotated = current_block;
  for (int y = 0; y < current_block.size; y++) {
    for (int x = 0; x < current_block.size; x++) {
      rotated.blocks[x][current_block.size - 1 - y] = current_block.blocks[y][x];
    }
  }

  return rotated;
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  Places the current Tetris block on the game grid, marking its cells as 
//  occupied. Checks for and clears any completed rows, then spawns a new 
//  block at the starting position. Ends the game if the new block cannot be 
//  placed due to collisions.
//
// INPUT PARAMETERS:
//  none
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  none
// 
//-----------------------------------------------------------------------------
void place_shape() {
  for (int y = 0; y < current_block.size; y++) {
    for (int x = 0; x < current_block.size; x++) {
      if (current_block.blocks[x][y]) {
        block_grid[selected_block_pos.x + x][selected_block_pos.y + y] = current_block.color;
      }
    }
  }
  clear_rows();

  current_block = SHAPES[rand() % (sizeof(SHAPES) / sizeof(SHAPES[0]))];
  selected_block_pos.x = (TETRIS_WIDTH - current_block.size) / 2;
  selected_block_pos.y = 0;

  if (!can_move(0, 0)) {
    game_over = true;
  }
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  Checks if the current Tetris block can be moved by the specified 
//  vertical (x_step) and horizontal (y_step) steps. This function verifies 
//  that the block does not collide with walls or other placed blocks.
//
// INPUT PARAMETERS:
//  x_step: The horizontal movement step (positive for down, negative for up).
//  y_step: The vertical movement step (positive for right, negative for left).
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  0 if the movement is blocked by the walls or other blocks.
//  1 if the block can move by the specified steps, 
// 
//-----------------------------------------------------------------------------
bool can_move(int8_t x_step, int8_t y_step) {
  for (int8_t y = 0; y < current_block.size; y++) {
    for (int8_t x = 0; x < current_block.size; x++) {
      if (current_block.blocks[x][y] != TETRIS_NO_BLOCK) {
        int8_t nx = selected_block_pos.x + x + x_step;
        int8_t ny = selected_block_pos.y + y + y_step;

        if (nx < 0 || nx >= TETRIS_WIDTH || ny >= TETRIS_HEIGHT || 
            (ny >= 0 && block_grid[nx][ny])) {
          return false;
        }
      }
    }
  }
  
  return true;
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  Attempts to move the current Tetris block by the specified vertical (x_step) 
//  and horizontal (y_step) steps. If the move is valid (no collisions), it 
//  updates the block's position and the game state/render state.
//
// INPUT PARAMETERS:
//  x_step: The horizontal movement step (positive for down, negative for up).
//  y_step: The vertical movement step (positive for right, negative for left).
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  bool: Returns true if the block was successfully moved, 
//        false if the move was blocked by walls or other blocks.
// 
//-----------------------------------------------------------------------------
bool move(int8_t x_step, int8_t y_step)
{
  bool movable = can_move(x_step, y_step);
  if(movable)
  {
    Vector2 previous_pos = selected_block_pos;

    selected_block_pos.x += x_step;
    selected_block_pos.y += y_step;
  }

  return movable;
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  Renders the "game over" screen by gradually clearing the block grid row by 
//  row. This function creates a visual effect by updating the screen with a 
//  delay between each row clearance.
//
// INPUT PARAMETERS:
//  none
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  none
//-----------------------------------------------------------------------------
void render_game_over() 
{
  for(uint8_t x = 0; x < TETRIS_HEIGHT; x++)
  {
    memset(block_grid[x], RED, sizeof(block_grid[0]));
    render_screen();
    sleep_ms(TETRIS_FILL_DELAY);
  }
}

//-----------------------------------------------------------------------------
// DESCRIPTION:
//  Renders the Tetris game screen by drawing each block on the grid. It updates
//  the screen to reflect the current state of the game, including the static 
//  blocks and the currently falling block. Different colors are used for the 
//  blocks based on their type.
//
// INPUT PARAMETERS:
//  none
//
// OUTPUT PARAMETERS:
//  none
//
// RETURN:
//  none
//-----------------------------------------------------------------------------
void render_screen()
{
  for (uint8_t y = 0; y < TETRIS_HEIGHT; y++) 
  {
    for (uint8_t x = 0; x < TETRIS_WIDTH; x++)
    {
      put_pixel_xy(x, TETRIS_HEIGHT - 1 - y, BLOCK_COLOR[block_grid[x][y]]);
    }
  }

  for (uint8_t y = 0; y < current_block.size; y++) {
    for (uint8_t x = 0; x < current_block.size; x++) {
      if (current_block.blocks[x][y] != TETRIS_NO_BLOCK) {
        put_pixel_xy(selected_block_pos.x + x, TETRIS_HEIGHT - 1 - (selected_block_pos.y + y), BLOCK_COLOR[current_block.color]);
      }
    }
  }

  write_buffer();
}
