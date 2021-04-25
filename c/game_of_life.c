/*
 Copyright 2021 Dom Routley

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define xlim 150
#define ylim 150
#define runlimit 370 // Apparently 370 is the max this can be set to...

void waitT (int secs) {
  int end = time(0) + secs;
  while (time(0) < end);
}

void print_board(_Bool board[xlim][ylim]){
  char symbol;
  printf("\33[2J"); // Clear the screen
  printf("\033[0;0H\n"); // Start the curser row 0, col 0
  for(int x=0; x<xlim; x++){
    // printf("|");
    for(int y=0; y<ylim; y++){
      if(board[x][y]){
        symbol = 'X';
      }else{
        symbol = '.';
      }
      printf("%c", symbol);
    }
    printf("\n");
  }
  waitT(1);
}

int neighbors_alive(_Bool board[xlim][ylim], int x, int y){
  int x_minus, y_minus, x_plus, y_plus;

  // Wrap dimensions for x axis
  switch (x){
  case 0:
    x_minus = xlim - 1;
    x_plus = x + 1;
    break;
  case xlim - 1:
    x_minus = x - 1;
    x_plus = 0;
    break;
  default:
    x_minus = x - 1;
    x_plus = x + 1;
    break;
  }

  // Wrap dimensions for y axis
  switch (y){
  case 0:
    y_minus = ylim - 1;
    y_plus = y + 1;
    break;
  case ylim - 1:
    y_minus = y - 1;
    y_plus = 0;
    break;
  default:
    y_minus = y - 1;
    y_plus = y + 1;
    break;
  }

  _Bool top_left = board[x_minus][y_minus];
  _Bool top_center = board[x_minus][y];
  _Bool top_right = board[x_minus][y_plus];
  _Bool left = board[x][y_minus];
  _Bool right = board[x][y_plus];
  _Bool lower_left = board[x_plus][y_minus];
  _Bool lower_center = board[x_plus][y];
  _Bool lower_right = board[x_plus][y_plus];

  return top_left + top_center + top_right + left + right + lower_left + lower_center + lower_right;
}

int duplicate(_Bool board[xlim][ylim], _Bool board_store[runlimit][xlim][ylim]){
  for(int i=0; i < runlimit; i++){
    int flag = 0;
    for(int x=0; x<xlim; x++){
      for(int y=0; y<ylim; y++){
        if(board_store[i][x][y] != board[x][y]){
          flag = 1; // Set divergent flag
          break; // Leave this column
        }
      }
      if(flag) break; // If divergent, break from board
    }
    if(!flag) return 1; // If board was left AND there is no divergent flag set, return duplicate = true
  }
  return 0;
}

int main(){
  // Start timing
  clock_t start, end;
  double delta_t;
  start = clock();

  _Bool board [xlim][ylim]; // Define board
  _Bool next_board[xlim][ylim];
  _Bool board_store[runlimit][xlim][ylim];
  srand(314); // Set random seed

  // TODO: combine these three loops

  // Clean board (set all coords to 0)
  for(int x=0; x<xlim; x++){
    for(int y=0; y<ylim; y++){
      board[x][y] = 0;
      next_board[x][y] = 0;
    }
  }

  // (150^2)/5 = 4500 = 20% of the board
  // This doesn't result in exactly 20% of the board, as the random coords might be the same
  for(int i=0; i<((xlim*ylim)/5); i++){
    int x = rand()%xlim;
    int y = rand()%ylim;
    board[x][y] = 1;
  }

  // Add seed board to store
  for(int x=0; x<xlim; x++){
    for(int y=0; y<ylim; y++){
      board_store[0][x][y] = board[x][y];
    }
  }

  int store_iterator = 1;
  int run_counter = 0;
  _Bool detected_loop = 0;
  while (!detected_loop){
    // Rules
    // Any live cell with fewer than two live neighbours dies, as if by underpopulation.
    // Any live cell with two or three live neighbours lives on to the next generation.
    // Any live cell with more than three live neighbours dies, as if by overpopulation.
    // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
    for(int x=0; x<xlim; x++){
      for(int y=0; y<ylim; y++){
        switch (neighbors_alive(board, x, y)){
        case 2:
          next_board[x][y] = board[x][y]; // Stay as you are
          break;
        case 3:
          next_board[x][y] = 1; // Live
          break;
        default: // By default, die
          next_board[x][y] = 0;
          break;
        }
      }
    }


    // Check if board exists in store, then store it if not
    detected_loop = duplicate(next_board, board_store);
    if(store_iterator == runlimit-1){
      store_iterator = 0;
    }else{
      store_iterator++;
    }

    for(int x=0; x<xlim; x++){
      for(int y=0; y<ylim; y++){
        board_store[store_iterator][x][y] = next_board[x][y];
      }
    }

    // Switch boards, we need board to be made the same as next_board
    // Would be more efficient to just swap pointers
    for (int x=0; x<xlim; x++){
      for(int y=0; y<ylim; y++){
        board[x][y] = next_board[x][y];
      }
    }

    run_counter++;
  }

  end = clock();
  delta_t = ((double) (end - start)) / CLOCKS_PER_SEC;
  double tps = run_counter/delta_t;

  printf("Ticks completed  : %d\n", run_counter);
  printf("Elapsed time (s) : %f\n", delta_t);
  printf("Ticks per second : %f\n", tps);
}
