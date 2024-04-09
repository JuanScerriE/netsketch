#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SNAKE_MAX_LEN 15 * 15

typedef enum { LEFT, RIGHT, UP, DOWN } Direction;

typedef struct {
  int body_len;
  Vector2 body_blocks[SNAKE_MAX_LEN];
  Direction direction;
  int block_size;
} Snake;

#define UPDATE_RATE 7

void reset_snake(Snake *snake, int x, int y) {
  snake->body_len = 1;
  snake->direction = UP;
  snake->body_blocks[0].x = x;
  snake->body_blocks[0].y = y;
}

void snake_left(Snake *snake, int step_size) {
  Vector2 temp = snake->body_blocks[0];
  snake->body_blocks[0].x -= step_size;

  for (int i = 0; i < snake->body_len - 1; i++) {
    Vector2 temp_2 = snake->body_blocks[i + 1];
    snake->body_blocks[i + 1] = temp;
    temp = temp_2;
  }
}

void snake_right(Snake *snake, int step_size) {
  Vector2 temp = snake->body_blocks[0];
  snake->body_blocks[0].x += step_size;

  for (int i = 0; i < snake->body_len - 1; i++) {
    Vector2 temp_2 = snake->body_blocks[i + 1];
    snake->body_blocks[i + 1] = temp;
    temp = temp_2;
  }
}

void snake_up(Snake *snake, int step_size) {
  Vector2 temp = snake->body_blocks[0];
  snake->body_blocks[0].y -= step_size;

  for (int i = 0; i < snake->body_len - 1; i++) {
    Vector2 temp_2 = snake->body_blocks[i + 1];
    snake->body_blocks[i + 1] = temp;
    temp = temp_2;
  }
}

void snake_down(Snake *snake, int step_size) {
  Vector2 temp = snake->body_blocks[0];
  snake->body_blocks[0].y += step_size;

  for (int i = 0; i < snake->body_len - 1; i++) {
    Vector2 temp_2 = snake->body_blocks[i + 1];
    snake->body_blocks[i + 1] = temp;
    temp = temp_2;
  }
}

Direction find_direction(Vector2 v1, Vector2 v2) {
  Vector2 dir_v;

  dir_v.x = v2.x - v1.x;
  dir_v.y = v2.y - v1.y;

  if (dir_v.x == 0) {
    if (dir_v.y > 0) {
      return UP;
    } else {
      return DOWN;
    }
  } else {
    if (dir_v.x > 0) {
      return RIGHT;
    } else {
      return LEFT;
    }
  }
}

void snake_grow(Snake *snake) {
  if (snake->body_len >= SNAKE_MAX_LEN)
    return;

  Direction direction;

  if (snake->body_len == 1) {
    direction = snake->direction;
  } else {
    direction = find_direction(snake->body_blocks[snake->body_len - 1],
                               snake->body_blocks[snake->body_len - 2]);
  }

  snake->body_len++;

  snake->body_blocks[snake->body_len - 1] =
      snake->body_blocks[snake->body_len - 2];

  switch (direction) {
  case LEFT:
    snake->body_blocks[snake->body_len - 1].x += snake->block_size;
    break;
  case RIGHT:
    snake->body_blocks[snake->body_len - 1].x -= snake->block_size;
    break;
  case UP:
    snake->body_blocks[snake->body_len - 1].y += snake->block_size;
    break;
  case DOWN:
    snake->body_blocks[snake->body_len - 1].y -= snake->block_size;
    break;
  }
}

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;
  const int centerWidth = screenWidth / 2;
  const int centerHeight = screenHeight / 2;

  // Snake Arena
  const int arenaSize = 15;
  const int arenaWidth = 300;
  const int arenaHeight = 300;
  const int arenaX = centerWidth - arenaWidth / 2;
  const int arenaY = centerHeight - arenaHeight / 2;

  InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

  SetTargetFPS(60);

  int step_size = arenaHeight / arenaSize;

  Snake snake;
  snake.body_len = 1;
  snake.body_blocks[0].x = centerWidth;
  snake.body_blocks[0].y = centerHeight;
  snake.block_size = step_size;
  snake.direction = UP;

  int frame_counter = 0;

  Vector2 size;
  size.x = snake.block_size;
  size.y = snake.block_size;

  Vector2 apple_location;
  (void)apple_location;

  bool grow_snake = false;
  bool snake_died = false;
  bool apple_present = false;

  // seed the random number generator with program start.
  // this is used for when you want to deal with
  srand(time(NULL));

  while (!WindowShouldClose()) {
    {
      if (IsKeyPressed(KEY_RIGHT) &&
          (snake.direction != LEFT || snake.body_len == 1))
        snake.direction = RIGHT;

      if (IsKeyPressed(KEY_LEFT) &&
          (snake.direction != RIGHT || snake.body_len == 1))
        snake.direction = LEFT;

      if (IsKeyPressed(KEY_UP) &&
          (snake.direction != DOWN || snake.body_len == 1))
        snake.direction = UP;

      if (IsKeyPressed(KEY_DOWN) &&
          (snake.direction != UP || snake.body_len == 1))
        snake.direction = DOWN;

      if (snake_died && IsKeyPressed(KEY_SPACE)) {
        reset_snake(&snake, centerWidth, centerHeight);
        snake_died = false;
      }
    }

    if (frame_counter % UPDATE_RATE == 0) {
      if (!apple_present) {
        double r_x = rand() / (double)RAND_MAX;
        double r_y = rand() / (double)RAND_MAX;
        (void)r_x;
        (void)r_y;
        /* printf("Value: %f\n", random); */
      }

      if (snake.direction == RIGHT)
        snake_right(&snake, step_size);
      if (snake.direction == LEFT)
        snake_left(&snake, step_size);
      if (snake.direction == UP)
        snake_up(&snake, step_size);
      if (snake.direction == DOWN)
        snake_down(&snake, step_size);

      // check if snake has hit the boundary
      {
        if (snake.body_blocks[0].x < arenaX ||
            snake.body_blocks[0].x > arenaX + arenaWidth)
          snake_died = true;

        if (snake.body_blocks[0].y < arenaY ||
            snake.body_blocks[0].y > arenaY + arenaHeight)
          snake_died = true;
      }

      // check if the snake has hit itself
      {
        Vector2 head = snake.body_blocks[0];
        for (int i = 1; i < snake.body_len; i++) {
          Vector2 body_block = snake.body_blocks[i];
          if (head.x == body_block.x && head.y == body_block.y)
            snake_died = true;
        }
      }

      if (grow_snake && !snake_died) {
        snake_grow(&snake);
        grow_snake = false;
      }

      frame_counter = 0;
    }

    BeginDrawing();
    {
      ClearBackground(RAYWHITE);
      if (snake_died) {
        // TODO: properly center text
        DrawText("Snake Died", centerWidth, centerHeight, 16,
                 RED); // Draw text (using default font)

      } else {
        for (int i = 0; i < snake.body_len; i++) {
          Vector2 pos;

          pos.x = snake.body_blocks[i].x - size.x / 2;
          pos.y = snake.body_blocks[i].y - size.y / 2;

          DrawRectangleV(pos, size, GREEN);
        }

        for (int y = arenaY; y <= arenaY + arenaHeight; y += step_size) {
          DrawLine(arenaX, y, arenaX + arenaWidth, y, RED);
        }

        for (int x = arenaX; x <= arenaX + arenaWidth; x += step_size) {
          DrawLine(x, arenaY, x, arenaY + arenaHeight, RED);
        }
      }
    }
    EndDrawing();

    frame_counter++;
  }

  CloseWindow();

  return 0;
}
