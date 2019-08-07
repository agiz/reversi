#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include <CL/cl.h>

//#include </opt/intel/impi/5.0.1.035/intel64/include/mpi.h> // FOR VIM TO STFU
#include <mpi.h> // THIS IS FOR COMPILING
// TODO: ALWAYS CHANGE BEFORE COMPILING!!!

#define CL_QUEUE_ORDER 0
// inorder/outoforder (?)

#define REPETITIONS 1
#define WORKGROUP_SIZE (512)
// nvidia tesla has 1024

static const size_t LS = WORKGROUP_SIZE;
static const size_t num_groups = ((8192 * 16 - 1) / LS + 1);
static const size_t GS = num_groups * LS;

#define MAX_SOURCE_SIZE 32768
// max character length of kernel

//Definiramo velikost igralnega polja
#define BOARD_HEIGHT 8
#define BOARD_WIDTH 8

//Mozne barve polja
#define BLACK (-1)
#define B 8
#define WHITE 1
#define W 0

#define MAX_DEPTH 3
//Maksimalna globina iskanja Minimax algoritma

#define MIN(x, y) (((y) > (x)) ? (x) : (y))

#define SET_MIN_MAX(x, y, min, max) \
  (((x) > (y)) ? ({ max = x; min = y; }) : ({ max = y; min = x; }))

#define PACK(a, player, x, y) \
  a[(player << 3) + (x)] |= (1 << (7 - (y))); \
  a[((1 - player ) << 3) + (x)] &= ~(1 << (7 - (y))) & 0xff; \
  a[((player + 2) << 3) + (y)] |= (1 << (7 - (x))); \
  a[((1 - player + 2) << 3) + (y)] &= ~(1 << (7 - (x))) & 0xff;

#define CPACK(a, player, s, x, y) \
  a[((player + ((s) << 1)) << 3) + (x)] |= (1 << (7 - (y))); \
  a[((1 - player + ((s) << 1)) << 3) + (x)] &= ~(1 << (7 - (y))) & 0xff; \
  a[((player + 2 - ((s) << 1)) << 3) + (y)] |= (1 << (7 - (x))); \
  a[((1 - player + 2 - ((s) << 1)) << 3) + (y)] &= ~(1 << (7 - (x))) & 0xff;

#define TILE(a, player, x, y) ((a[((player << 3) + (x))] >> (7 - (y))) & 0x1)

#define COPY_BOARD(boards, board, i) \
  boards[((i) << 3) + 0] = *((uint32_t *)board + 0); \
  boards[((i) << 3) + 1] = *((uint32_t *)board + 1); \
  boards[((i) << 3) + 2] = *((uint32_t *)board + 2); \
  boards[((i) << 3) + 3] = *((uint32_t *)board + 3); \
  boards[((i) << 3) + 4] = *((uint32_t *)board + 4); \
  boards[((i) << 3) + 5] = *((uint32_t *)board + 5); \
  boards[((i) << 3) + 6] = *((uint32_t *)board + 6); \
  boards[((i) << 3) + 7] = *((uint32_t *)board + 7);

#define VALID 1

//Podatkovne strukture potrebne pri postavljanju novega ploscka

static const uint8_t n_of_bits[256] = {
  0, 1, 1, 2, 1, 2, 2, 3,
  1, 2, 2, 3, 2, 3, 3, 4,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6,
  4, 5, 5, 6, 5, 6, 6, 7,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6,
  4, 5, 5, 6, 5, 6, 6, 7,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6,
  4, 5, 5, 6, 5, 6, 6, 7,
  3, 4, 4, 5, 4, 5, 5, 6,
  4, 5, 5, 6, 5, 6, 6, 7,
  4, 5, 5, 6, 5, 6, 6, 7,
  5, 6, 6, 7, 6, 7, 7, 8
};

static size_t ctotal = 5;

#define SHIFT (sizeof(int32_t) * CHAR_BIT - 1)

uint8_t balance(uint32_t *S, uint8_t *o, size_t n) {
  size_t i, j, k, c = 0;
  const uint32_t total = S[0] + S[1] + S[2] + S[3] + S[4] + S[5] + S[6] + S[7];
  uint32_t sum[2] = {S[0], total - S[0]};
  uint8_t temp_o;
  o[0] = 2; o[1] = 3; o[2] = 4;
  o[3] = 5; o[4] = 6; o[5] = 7;

  // singles
  if (sum[0] == sum[1]) { return 0; }
  if (total - S[1] == S[1]) { return 6; }

  int32_t diff = sum[0] - sum[1];
  int32_t mask = (diff) >> SHIFT;
  uint32_t udiff, min_diff = (diff + mask) ^ mask;

  diff = total - S[1] - S[1];
  mask = (diff) >> SHIFT;
  udiff = (diff + mask) ^ mask;
  if (udiff < min_diff) {
    min_diff = udiff;
    c = 6;
  }

  // tuples S[0] first
  for (i=2; i<n; i++) {
    sum[0] = S[0] + S[i];
    sum[1] = total - sum[0];
    diff = sum[1] - sum[0];
    mask = (diff) >> SHIFT;
    udiff = (diff + mask) ^ mask;
    if (udiff < min_diff) {
      min_diff = udiff;
      c = 1;

      o[0] = i;

      if (udiff == 0) {
        for (i=1, j=2; i<6; i++, j++) {
          if (o[0] == j) { j++; }
          o[i] = j;
        }
        return c;
      }
    }
  }
  // tuples S[1] first
  for (i=2; i<n; i++) {
    sum[0] = S[1] + S[i];
    sum[1] = total - sum[0];
    diff = sum[1] - sum[0];
    mask = (diff) >> SHIFT;
    udiff = (diff + mask) ^ mask;
    if (udiff < min_diff) {
      min_diff = udiff;
      c = 5;

      o[0] = i;

      if (udiff == 0) {
        for (i=1, j=2; i<6; i++, j++) {
          if (o[0] == j) { j++; }
          o[i] = j;
        }
        temp_o = o[5]; o[5] = o[0]; o[0] = temp_o;
        return c;
      }
    }
  }

  // triples S[0] first
  for (i=2; i<n; i++) {
    for (j=i + 1; j<n; j++) {
      sum[0] = S[0] + S[i] + S[j];
      sum[1] = total - sum[0];
      diff = sum[1] - sum[0];
      mask = (diff) >> SHIFT;
      udiff = (diff + mask) ^ mask;
      if (udiff < min_diff) {
        min_diff = udiff;
        c = 2;

        o[0] = i;
        o[1] = j;

        if (udiff == 0) {
          for (i=2, j=2; i<6; i++, j++) {
            if (o[0] == j) { j++; }
            if (o[1] == j) { j++; }
            o[i] = j;
          }
          return c;
        }
      }
    }
  }
  // triples S[1] first
  for (i=2; i<n; i++) {
    for (j=i + 1; j<n; j++) {
      sum[0] = S[1] + S[i] + S[j];
      sum[1] = total - sum[0];
      diff = sum[1] - sum[0];
      mask = (diff) >> SHIFT;
      udiff = (diff + mask) ^ mask;
      if (udiff < min_diff) {
        min_diff = udiff;
        c = 4;

        o[0] = i;
        o[1] = j;

        if (udiff == 0) {
          for (i=2, j=2; i<6; i++, j++) {
            if (o[0] == j) { j++; }
            if (o[1] == j) { j++; }
            o[i] = j;
          }
          temp_o = o[5]; o[5] = o[0]; o[0] = temp_o;
          temp_o = o[4]; o[4] = o[1]; o[1] = temp_o;
          return c;
        }
      }
    }
  }


  // quads S[0] first
  for (i=2; i<n; i++) {
    for (j=i + 1; j<n; j++) {
      for (k=j + 1; k<n; k++) {
        sum[0] = S[0] + S[i] + S[j] + S[k];
        sum[1] = total - sum[0];
        diff = sum[1] - sum[0];
        mask = (diff) >> SHIFT;
        udiff = (diff + mask) ^ mask;
        if (udiff < min_diff) {
          min_diff = udiff;
          c = 3;

          o[0] = i;
          o[1] = j;
          o[2] = k;

          if (udiff == 0) {
            for (i=3, j=2; i<6; i++, j++) {
              if (o[0] == j) { j++; }
              if (o[1] == j) { j++; }
              if (o[2] == j) { j++; }
              o[i] = j;
            }
            return c;
          }
        }
      }
    }
  }
  // quads S[1] first
  for (i=2; i<n; i++) {
    for (j=i + 1; j<n; j++) {
      for (k=j + 1; k<n; k++) {
        sum[0] = S[1] + S[i] + S[j] + S[k];
        sum[1] = total - sum[0];
        diff = sum[1] - sum[0];
        mask = (diff) >> SHIFT;
        udiff = (diff + mask) ^ mask;
        if (udiff < min_diff) {
          min_diff = udiff;
          c = 9;

          o[0] = i;
          o[1] = j;
          o[2] = k;

          if (udiff == 0) {
            for (i=3, j=2; i<6; i++, j++) {
              if (o[0] == j) { j++; }
              if (o[1] == j) { j++; }
              if (o[2] == j) { j++; }
              o[i] = j;
            }
            temp_o = o[5]; o[5] = o[0]; o[0] = temp_o;
            temp_o = o[4]; o[4] = o[1]; o[1] = temp_o;
            temp_o = o[3]; o[3] = o[2]; o[2] = temp_o;
            c = 3;
            return c;
          }
        }
      }
    }
  }

  if (c == 0 || c == 6) {
  }
  else if (c == 1 || c == 5) {
    for (i=1, j=2; i<6; i++, j++) {
      if (o[0] == j) { j++; }
      o[i] = j;
    }
    if (c == 5) {
      temp_o = o[5]; o[5] = o[0]; o[0] = temp_o;
    }
  }
  else if (c == 2 || c == 4) {
    for (i=2, j=2; i<6; i++, j++) {
      if (o[0] == j) { j++; }
      if (o[1] == j) { j++; }
      o[i] = j;
    }
    if (c == 4) {
      temp_o = o[5]; o[5] = o[0]; o[0] = temp_o;
      temp_o = o[4]; o[4] = o[1]; o[1] = temp_o;
    }
  }
  else {
    for (i=3, j=2; i<6; i++, j++) {
      if (o[0] == j) { j++; }
      if (o[1] == j) { j++; }
      if (o[2] == j) { j++; }
      o[i] = j;
    }
    if (c == 9) {
      temp_o = o[5]; o[5] = o[0]; o[0] = temp_o;
      temp_o = o[4]; o[4] = o[1]; o[1] = temp_o;
      temp_o = o[3]; o[3] = o[2]; o[2] = temp_o;
      c = 3;
    }
  }

  return c;
}

//Inicializiramo zacetno igralno polje
void init_board(uint8_t *board) {
  for (int i=0; i<32; i++) {
    board[i] = 0;
  }

  PACK(board, 1, 3, 2)

  PACK(board, 1, 3, 3)
  PACK(board, 1, 3, 4)
  PACK(board, 1, 4, 3)
  PACK(board, 0, 4, 4)
}

//Izrisi igralno polje
void print_board(uint8_t *board) {
  printf("  ");

  for (int j = 0; j < BOARD_WIDTH; j++) { printf("-%d", j); }
  printf("\n");

  for (int i = 0; i < BOARD_HEIGHT; i++) {
    printf("%d| ",i);
    for (int j = 0; j < BOARD_WIDTH; j++) {
      char c = ' ';

      if (TILE(board, 0, i, j)) { c = 'O'; }
      else if (TILE(board, 1, i, j)) { c = 'X'; }

      printf("%c ",c);
    }
    printf("\b|\n");
  }
  printf("  ");

  for (int j = 0; j < BOARD_WIDTH; j++) { printf("=="); }
  printf("\n");
}

static const uint8_t ffs_lu[32] = {
  0, 1, 2, 1, 3, 1, 2, 1,
  4, 1, 2, 1, 3, 1, 2, 1,
  5, 1, 2, 1, 3, 1, 2, 1,
  4, 1, 2, 1, 3, 1, 2, 1
};

static const uint8_t clz_lu[32] = {
  8, 7, 6, 6, 5, 5, 5, 5,
  4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3
};

int is_move_valid(int player, uint8_t *board, int row, int col) {
  if (((board[row] | board[B + row]) >> (7 - col)) & 0x1) {
    return 0;
  }

  player = 1 - ((1 + player) >> 1);
  uint8_t i, d, f, s, p, t, v, tmp;
  int8_t u;

  for (i = 0; i < 2; i++) {
    s = board[((player + (i << 1)) << 3) + row];
    // """row of players discs"""

    p = board[(((1 - player + (i << 1))) << 3) + row];
    // """row of opponents discs"""

    if (col < 6 && ((p >> (6 - col)) & 0x1)) {

      t = s & (0x7f >> (1 + col));

      v = (t >> 5) & 0x1;
      // """MSB"""

      u = clz_lu[t & 0x1f];
      u = 7 - ((u & (~(-v) & 0xf)) | (v << 1));

      if (u > -1 &&
          (((p | s) >> u) & (0x7f >> (col + u))) == (0x7f >> (col + u))) {
        return 1;
      }
    }

    if ((col > 1 && ((p >> (8 - col)) & 0x1))) {
      t = s >> (9 - col);
      u = ffs_lu[t & 0x1f];

      if (u == 0 && t == 32) {
        u = 6;
      }
      else if (u == 0) {
        tmp = row;
        row = col;
        col = tmp;
        continue;
      }

      if ((((p | s) >> (9 - col)) & (0xff >> (8 - u))) == (0xff >> (8 - u))) {
        return 1;
      }
    }

    tmp = row;
    row = col;
    col = tmp;
  }

  SET_MIN_MAX(row, col, f, d);

  i = 1;
  // # pristevamo 1 po diagonali
  if (d < 6) {
    v = TILE(board, (1 - player + 2 - (i << 1)), row + 1, col + 1);

    if (v) {
      for (uint8_t j = 2; j < 8 - d; j++) {
        if (TILE(board, (player + 2 - (i << 1)), row + j, col + j)) {
          return 1;
        }
        if (TILE(board, (1 - player + 2 - (i << 1)), row + j, col + j) == 0) {
          // print "top left to bottom right failed."
          break;
        }
      }
    }
  }

  if (f > 1) {
    v = TILE(board, (1 - player + 2 - (i << 1)), row - 1, col - 1);

    if (v) {
      for (uint8_t j = 2; j < f + 1; j++) {
        if (TILE(board, (player + 2 - (i << 1)), row - j, col - j)) {
          return 1;
        }
        if (TILE(board, (1 - player + 2 - (i << 1)), row - j, col - j) == 0) {
          // print "bottom right to top left failed."
          break;
        }
      }
    }
  }

  i = 1;

  //print 'kontra diagonala 1'
  if (col < 6 && row > 1) {
    v = TILE(board, (1 - player + 2 - (i << 1)), row - 1, col + 1);

    uint8_t l = MIN(row + col, 7) - col + 1;

    if (v) {
      for (uint8_t j = 2; j < l; j++) {
        if (TILE(board, (player + 2 - (i << 1)), row - j, col + j)) {
          return 1;
        }
        if (TILE(board, (1 - player + 2 - (i << 1)), row - j, col + j) == 0) {
          // print "bottom left to top right failed."
          break;
        }
      }
    }
  }

  //print 'kontra diagonala 2'
  if (col > 1 && row < 6) {
    v = TILE(board, (1 - player + 2 - (i << 1)), row + 1, col - 1);

    uint8_t l = MIN(row + col, 7) - row + 1;

    if (v) {
      for (uint8_t j = 2; j < l; j++) {
        if (TILE(board, (player + 2 - (i << 1)), row + j, col - j)) {
          return 1;
        }
        if (TILE(board, (1 - player + 2 - (i << 1)), row + j, col - j) == 0) {
          // print "bottom left to top right failed."
          break;
        }
      }
    }
  }

  // print 'not found'
  return 0;
}

void place_piece(int row, int col, int player, uint8_t *board) {
  player = 1 - ((1 + player) >> 1);

  uint8_t i, d, f, s, p, t, v, tmp;
  int8_t u;

  for (i = 0; i < 2; i++) {
    s = board[((player + (i << 1)) << 3) + row];
    // """row of players discs"""

    p = board[(((1 - player + (i << 1))) << 3) + row];
    // """row of opponents discs"""

    // looking right
    if (col < 6 && ((p >> (6 - col)) & 0x1)) {

      t = s & (0x7f >> (1 + col));

      v = (t >> 5) & 0x1;
      // """MSB"""

      u = clz_lu[t & 0x1f];
      u = 7 - ((u & (~(-v) & 0xf)) | (v << 1));

      if (u > -1 &&
          (((p | s) >> u) & (0x7f >> (col + u))) == (0x7f >> (col + u))) {
        // barvanje
        for (uint8_t c = col; c < 7 - u; c++) {
          CPACK(board, player, i, row, c);
        }
      }
    }

    // looking left
    if ((col > 1 && ((p >> (8 - col)) & 0x1))) {
      t = s >> (9 - col);
      u = ffs_lu[t & 0x1f];

      if (u == 0 && t == 32) {
        u = 6;
      }
      else if (u == 0) {
        tmp = row;
        row = col;
        col = tmp;
        continue;
      }

      if ((((p | s) >> (9 - col)) & (0xff >> (8 - u))) == (0xff >> (8 - u))) {
        // barvanje
        for (uint8_t c = col - u; c < col + 1; c++) {
          CPACK(board, player, i, row, c);
        }
      }
    }

    tmp = row;
    row = col;
    col = tmp;
  }

  SET_MIN_MAX(row, col, f, d);

  i = 1;
  // # pristevamo 1 po diagonali
  if (d < 6) {
    uint8_t takeover = 0;
    v = TILE(board, (1 - player + 2 - (i << 1)), row + 1, col + 1);

    if (v) {
      for (uint8_t j = 2; j < 8 - d; j++) {
        if TILE(board, (player + 2 - (i << 1)), row + j, col + j) {
          takeover = j;
          break;
        }
        if (TILE(board, (1 - player + 2 - (i << 1)), row + j, col + j) == 0) {
          // print "top left to bottom right failed."
          break;
        }
      }
      // barvanje
      if (takeover) {
        for (uint8_t c = 0; c < takeover; c++) {
          PACK(board, player, row + c, col + c);
        }
      }
    }
  }

  if (f > 1) {
    uint8_t takeover = 0;
    v = TILE(board, (1 - player + 2 - (i << 1)), row - 1, col - 1);

    if (v) {
      for (uint8_t j = 2; j < f + 1; j++) {
        if (TILE(board, (player + 2 - (i << 1)), row - j, col - j)) {
          takeover = j;
          break;
        }
        if (TILE(board, (1 - player + 2 - (i << 1)), row - j, col - j) == 0) {
          // print "bottom right to top left failed."
          break;
        }
      }
      // barvanje
      if (takeover) {
        for (uint8_t c = 0; c < takeover; c++) {
          PACK(board, player, row - c, col - c);
        }
      }
    }
  }

  i = 1;

  //print 'kontra diagonala 1'
  if (col < 6 && row > 1) {
    uint8_t takeover = 0;
    v = TILE(board, (1 - player + 2 - (i << 1)), row - 1, col + 1);

    uint8_t l = MIN(row + col, 7) - col + 1;

    if (v) {
      for (uint8_t j = 2; j < l; j++) {
        if (TILE(board, (player + 2 - (i << 1)), row - j, col + j)) {
          takeover = j;
          break;
        }
        if (TILE(board, (1 - player + 2 - (i << 1)), row - j, col + j) == 0) {
          // print "bottom left to top right failed."
          break;
        }
      }
      // barvanje
      if (takeover) {
        for (uint8_t c = 0; c < takeover; c++) {
          PACK(board, player, row - c, col + c);
        }
      }
    }
  }

  //print 'kontra diagonala 2'
  if (col > 1 && row < 6) {
    uint8_t takeover = 0;
    v = TILE(board, (1 - player + 2 - (i << 1)), row + 1, col - 1);

    uint8_t l = MIN(row + col, 7) - row + 1;

    if (v) {
      for (uint8_t j = 2; j < l; j++) {
        if TILE(board, (player + 2 - (i << 1)), row + j, col - j) {
          takeover = j;
          break;
        }
        if (TILE(board, (1 - player + 2 - (i << 1)), row + j, col - j) == 0) {
          // print "bottom left to top right failed."
          break;
        }
      }
      // barvanje
      if (takeover) {
        for (uint8_t c = 0; c < takeover; c++) {
          PACK(board, player, row + c, col - c);
        }
      }
    }
  }
}

int evaluate_board(uint8_t *board) {
  int i, sum = 0;
  // Izracunamo razliko med stevilom belih in crnih plosckov
  for (i = 0; i < BOARD_HEIGHT; i++) {
    sum += n_of_bits[board[i]];
  }

  return sum;
}

//stack minimax
void gen_boards(uint8_t *board, int player, int max_h, int h,
    int row, int col, uint32_t *pb, uint32_t *bi) {
  // temporary stack structures
  uint8_t i, j, is[max_h + 1], js[max_h + 1], moved[max_h + 1], br = 0;
  moved[0] = 0; is[0] = 0; js[0] = 0;

  uint32_t tmp_b[8 * (max_h + 1)];
  //Nastavimo najmanjse mozno zacetno stevilo tock

  place_piece(row, col, player, board);
  //Postavimo ploscek

  uint8_t *pboard = board;
  // pointer to the current board.

  while (1) {
    // Pogledamo ce smo ze prisli do najvecje mozne globine rekurzije
    if (h > max_h) {
      h--;
      // reduce one level.

      player = -player;
      pboard = (uint8_t *)&tmp_b[(h - 1) << 3];
    }

    // Generiramo vse mozne poteze nasprotnega
    // igralca in se spustimo en nivo nizje
    for (i = is[h]; i < BOARD_HEIGHT; i++) {
      for (j = js[h]; j < BOARD_WIDTH; j++) {

        //Preverimo ce je nasprotnikova poteza veljavna
        if (is_move_valid(-player, pboard, i, j)) {
          moved[h] = 1;

          is[h] = i; js[h] = j + 1; player = -player;

          if (h + 1 > max_h) {
            // copy board to boards for parallel
            COPY_BOARD(pb, pboard, *bi);
            pboard = (unsigned char *)&pb[*bi << 3];
            place_piece(i, j, player, pboard);
            *bi = *bi + 1;
          }
          else {
            COPY_BOARD(tmp_b, pboard, h);
            pboard = (uint8_t *)&tmp_b[h << 3];
            place_piece(i, j, player, pboard);
          }

          h++;
          moved[h] = 0; is[h] = 0; js[h] = 0;

          //goto cont_while;
          br = 1; break;
        }
      }
      if (br) { break; }
      else { js[h] = 0; }
    }

    if (!br) {
      h--;
      // take off the stack

      if (h == -1) { return; }
      else if (h == 0) { pboard = board; }
      else { pboard = (uint8_t *)&tmp_b[(h - 1) << 3]; }

      // continue, where we left...
      player = -player;
    }
    br = 0;
  }
}

//stack minimax
int re_mm(uint8_t *board, int player, int max_h,
    int h, int32_t *pscore, uint32_t *bi) {
  int32_t i, j, score[max_h + 1];
  score[0] = -INT_MAX * player;

  // temporary stack structures
  uint8_t is[max_h + 1], js[max_h + 1], moved[max_h + 1], br = 0;
  moved[0] = 0; is[0] = 0; js[0] = 0;

  uint32_t tmp_b[8 * (max_h + 1)];
  //Nastavimo najmanjse mozno zacetno stevilo tock

  uint8_t *pboard = board;
  // pointer to the current board.

  while (1) {
    // Pogledamo ce smo ze prisli do najvecje mozne globine rekurzije
    if (h > max_h) {
      score[h] = pscore[*bi];
      *bi = *bi + 1;

      h--;
      // reduce one level.

      player = -player;
      pboard = (uint8_t *)&tmp_b[(h - 1) << 3];

      if (score[h + 1] * player > score[h] * player) {
        score[h] = score[h + 1];
      }
    }

    // Generiramo vse mozne poteze nasprotnega
    // igralca in se spustimo en nivo nizje
    for (i = is[h]; i < BOARD_HEIGHT; i++) {
      for (j = js[h]; j < BOARD_WIDTH; j++) {

        //Preverimo ce je nasprotnikova poteza veljavna
        if (is_move_valid(-player, pboard, i, j)) {
          moved[h] = 1;

          is[h] = i; js[h] = j + 1; player = -player;

          if (h + 1 > max_h) {
          }
          else {
            COPY_BOARD(tmp_b, pboard, h);
            pboard = (uint8_t *)&tmp_b[h << 3];
            place_piece(i, j, player, pboard);
          }

          h++;
          score[h] = -INT_MAX * player;
          moved[h] = 0; is[h] = 0; js[h] = 0;

          //goto cont_while;
          br = 1; break;
        }
      }
      if (br) { break; }
      else { js[h] = 0; }
    }

    if (!br) {
      // Ce ni bilo mozno narediti nobene poteze
      // evalviramo igralno polje in koncamo
      if (!moved[h]) { score[h] = evaluate_board(pboard); }

      h--;
      // take off the stack

      if (h == -1) { return score[0]; }
      else if (h == 0) { pboard = board; }
      else { pboard = (uint8_t *)&tmp_b[(h - 1) << 3]; }

      // continue, where we left...
      player = -player;

      if (score[h + 1] * player > score[h] * player) {
        score[h] = score[h + 1];
      }
      //
      //cont_while:;
    }
    br = 0;
  }

  printf("\nNO NO NO NO NO NO NO\n");
  fflush(stdout);
  return 0;
  // This never happens.
}

#define COPY_HOST_PTR_RO (CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR)

void par_mm2(uint32_t *pb, uint32_t board_n, int *score, cl_context ctx,
    cl_command_queue cq, cl_program program, cl_kernel kernel) {
  const uint32_t bs = board_n * sizeof(uint32_t);
  cl_int r;
  // error information

  cl_mem t_board, h_score;
  t_board = clCreateBuffer(ctx, COPY_HOST_PTR_RO, 8 * bs, pb, &r);
  // context, type, size, host buffer ptr, error

  h_score = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, bs, NULL, &r);
  // output buffer

  r |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&t_board);
  r |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&h_score);
  r |= clSetKernelArg(kernel, 2, sizeof(cl_int), (void *)&board_n);
  // kernel, arg index, size, data ptr

  r = clEnqueueNDRangeKernel(cq, kernel, 1, NULL, &GS, &LS, 0, NULL, NULL);
  // type, kernel, # of dims, global work offset, global thread ptr,
  // local thread ptr, # of events, event list, id of this event

  r = clEnqueueReadBuffer(cq, h_score, CL_TRUE, 0, bs, score, 0, NULL, NULL);
  // read buffer from target, offset, event params...

  r = clReleaseMemObject(h_score);
  r = clReleaseMemObject(t_board);
}

//Preverimo ce lahko trenutni igralec postavi svoj ploscek
int can_player_move(int player, uint8_t *board) {
  for (int i = 0; i < BOARD_HEIGHT; i++) {
    for (int j = 0; j < BOARD_WIDTH; j++) {
      if (is_move_valid(player, board, i, j)) {
        return VALID;
      }
    }
  }

  return 0;
}

//Preverimo ce je ze konec igre
int is_game_over(uint8_t *board) {
  //Ce noben od igralcev ne more narediti poteze je igre konec
  return (!can_player_move(WHITE, board) && !can_player_move(BLACK, board));
}

//Racunalniski igralec
int re_computer_move(uint8_t *board, int player) {
  int row = -1, col = -1, score = -INT_MAX * player;

  //Preverimo ce je ze konec igre
  if (is_game_over(board)) { return 0; }
  uint32_t tb[8], bi = 0;
  int tscore;

  //Generiramo in izvedemo vse mozne poteze racunalnika
  for (int i = 0; i < BOARD_HEIGHT; i++) {
    for (int j = 0; j < BOARD_WIDTH; j++) {
      if (is_move_valid(player, board, i, j)) {

        COPY_BOARD(tb, board, 0);
        place_piece(i, j, player, (uint8_t *)tb);

        tscore = re_mm((uint8_t *)tb, player, MAX_DEPTH, 0, NULL, &bi);
        //Najdemo najboljso potezo, klicemo s trenutno globino 1.

        if (tscore*player > score*player) {
          score = tscore; row = i; col = j;
        }
      }
    }
  }

  //Izvedemo najbolje ocenjeno potezo
  if (row >= 0 && col >= 0) {
    place_piece(row, col, player, board);
    ctotal++;
  }
  return 1;
}

//Racunalniski igralec
int re_computer_move2(uint8_t *board,
    int player, uint8_t i, int32_t *pscore, int *rcs) {
  int row = -1, col = -1, score = -INT_MAX * player;

  //Preverimo ce je ze konec igre
  if (is_game_over(board)) { return 0; }
  uint32_t tb[8];
  uint32_t bi = 0;

  //Generiramo in izvedemo vse mozne poteze racunalnika
  for (uint8_t j = 0; j < BOARD_WIDTH; j++) {
    if (is_move_valid(player, board, i, j)) {

      COPY_BOARD(tb, board, 0);
      place_piece(i, j, player, (uint8_t *)tb);

      int tscore = re_mm((uint8_t *)tb, player, MAX_DEPTH, 0, pscore, &bi);
      //Najdemo najboljso potezo, klicemo s trenutno globino 1.

      if (tscore*player > score*player) {
        score = tscore; row = i; col = j;
      }
    }
  }

  rcs[0] = row; rcs[1] = col; rcs[2] = score;

  return 1;
}

//Racunalniski igralec
int generate_computer_move2(uint8_t *board,
    int player, uint8_t i, uint32_t *pb, uint32_t *bi) {
  //Preverimo ce je ze konec igre
  if (is_game_over(board)) { return 0; }
  uint32_t tb[8];

  //Generiramo in izvedemo vse mozne poteze racunalnika
  for (uint8_t j = 0; j < BOARD_WIDTH; j++) {
    if (is_move_valid(player, board, i, j)) {
      COPY_BOARD(tb, board, 0);
      gen_boards((uint8_t *)tb, player, MAX_DEPTH, 0, i, j, pb, bi);
    }
  }

  return 1;
}

//Nakljucni racunalniski igralec
int computer_random_move(uint8_t *board, int player, int max_depth=0) {
  int row = -1, col = -1;

  //Preverimo ce je ze konec igre
  if (is_game_over(board)) { return 0; }

  struct move {
    int row;
    int col;
  };
  struct move available_moves[64];

  //Generiramo vse mozne poteze racunalnika
  int n_moves = 0;
  for (int i = 0; i < BOARD_HEIGHT; i++) {
    for (int j = 0; j < BOARD_WIDTH; j++) {
      if (is_move_valid(player, board, i, j)) {
        available_moves[n_moves].row = i;
        available_moves[n_moves].col = j;
        n_moves++;
      }
    }
  }

  //Izberemo nakljucno potezo
  if (n_moves > 0) {
    int move_index = rand() % n_moves;
    row = available_moves[move_index].row;
    col = available_moves[move_index].col;
    if (row >= 0 && col >= 0) {
      place_piece(row, col, player, board);
      ctotal++;
    }
  }
  return 1;
}

int no_moves_left(uint32_t *b) {
  return (b[0] | b[1] | b[2] | b[3] | b[4] | b[5] | b[6] | b[7]);
}

#define ROOT 0
#define GPU0 0
#define GPU1 1
#define CW MPI_COMM_WORLD

int main(int argc, char *argv[]) {
  double t_start, t_end;
  uint8_t board[32] = {0};
  int i, player1 = BLACK, player2 = WHITE;
  //Player X, O

  //
  // MPI variables
  //

  int rank;
  // rank (oznaka) procesa

  int proc_count;
  // stevilo procesov

  int tag = 0;
  // zaznamek sporocila

  MPI_Status s;
  // status sprejema

  MPI_Init(&argc, &argv);
  // inicializacija MPI okolja

  MPI_Comm_rank(CW, &rank);
  // poizvedba po ranku procesa

  MPI_Comm_size(CW, &proc_count);
  // poizvedba po stevilu procesov

  //
  // OpenCL variables
  //

  cl_int r = NULL;
  // error information

  cl_context ctx = NULL;
  cl_command_queue cq = NULL;
  cl_program prog = NULL;
  cl_kernel kernel = NULL;

  if (rank > 1) {}
  else {
    // read kernel from file
    FILE *fp; char *code; size_t source_size;
    fp = fopen("minimax.cl", "r");
    if (!fp) { fprintf(stderr, ":-(#\n"); exit(1); }
    code = (char*) malloc(MAX_SOURCE_SIZE);
    source_size = fread(code, 1, MAX_SOURCE_SIZE, fp);
    code[source_size] = '\0';
    fclose(fp);

    // platform info
    cl_platform_id  pltf_id[10];
    cl_uint ret_num_platforms;
    r = clGetPlatformIDs(10, pltf_id, &ret_num_platforms);

    cl_device_id dev_id[10];
    // device info

    cl_uint n_dev;
    // store number of devices

    r = clGetDeviceIDs(pltf_id[0], CL_DEVICE_TYPE_GPU, 10, dev_id, &n_dev);

    ctx = clCreateContext(NULL, 1, &dev_id[rank], NULL, NULL, &r);
    // platform (default NULL), # devices, device ptr, err, xtra args, err id

    cq = clCreateCommandQueue(ctx, dev_id[rank], CL_QUEUE_ORDER, &r);

    prog = clCreateProgramWithSource(ctx, 1, (const char **)&code, NULL, &r);
    // context, # of code ptrs, code ptrs, null terminated code string, err id

    r = clBuildProgram(prog, 1, &dev_id[rank], NULL, NULL, NULL);
    // build: program, # devices, devices, build opts, funciton ptr, user args

    kernel = clCreateKernel(prog, "minimax", &r);
    // program, name of kernel, error
  }

  if (rank == ROOT) {
    init_board(board);
    // 2 * 64 bits for board

    //int seed = time(NULL);
    //printf("seed: %d\n", seed);
    //srand(seed);
    srand(1); // dela is_move_valid
    //printf("-------START-------\n");
    //print_board(board);

    //Izvajamo poteze dokler ni konec igre
  }
  t_start = MPI_Wtime();

  uint32_t pb[8192 << 8];
  // parallel boards

  uint32_t bi;
  // number of generated boards for each slave

  int score[8192 << 5];
  // scores for updated game tree

  uint32_t rct[2];
  // rank - count tupple for each slave

  uint32_t rc[8];
  // rank count - number of boards generated per slave

  uint8_t p[8];
  // partition

  uint8_t t;
  // p split cutoff

  uint32_t board_count;
  // sum of all generated boards by all processes

  uint32_t off;
  // offset of pb array (where to store boards received from slaves)

  uint8_t dr[8];
  // dispatch rules - tell each slave whom to send generated boards

  int rcs[4];
  // row, column, socre of the best score for each slave

  int row, col, best_score;
  // for root

  while (1) {
    bi = 0; rc = {0}; p = {0}; t = proc_count + 1;
    board_count = 0; off = 0; dr = {0};
    rcs[0] = -1; rcs[1] = -1; rcs[2] = -INT_MAX * player1;
    row = -1; col = -1; best_score = -INT_MAX * player1;

    if (rank == ROOT) {
      if (!computer_random_move(board, player2, 1)) {

        //Izpisemo rezultat
        printf("-----GAME OVER-----\n");
        print_board(board);
        int whites = evaluate_board(board);
        printf("SCORE: Player X:%zu Player O:%d\n", ctotal - whites, whites);

        board = {0};
        // sign for no more available moves for both players

        //break;
        // This break is "ok" game terminating-wise because it
        // checks that both players cannot make any more moves.
      }
    }

    MPI_Bcast((uint32_t *)board, 8, MPI_UNSIGNED, 0, CW);

    if (no_moves_left((uint32_t *)board) == 0) {
      // There are no more legal moves for any player.
      if (rank == ROOT) {
        //printf("[*] no more legal moves #1\n");
        //fflush(stdout);
      }
      break;
    }

    if (!generate_computer_move2(board, player1, rank, pb, &bi)) {
      // THIS NEVER HAPPENS
      break;
    }

    // reduce all COUNT of generated boards to root
    if (rank == ROOT) {
      rc[ROOT] = bi;
      board_count = bi;
      for (i = 1; i < proc_count; i++) {
        MPI_Recv(rct, 2, MPI_UNSIGNED, i, tag, CW, &s);
        rc[rct[0]] = rct[1];
        board_count += rct[1];
      }

      if (board_count == 0) {
        // signal that there are no more valid moves for both players
        for (i = 0; i < proc_count; i++) {
          dr[i] = proc_count + 1;
        }
        p[proc_count - 1 - GPU1] = proc_count + 1;
        // signal no more available moves to GPU1
      }
      else {
        t = balance(rc, p, 8);
        // construct partition (greedy), with bias for gpu proc 0 and 1.

        for (i = 0; i < t; i++) {
          dr[p[i]] = GPU0;
        }

        for (i = t; i < proc_count - 2; i++) {
          dr[p[i]] = GPU1;
        }

        p[proc_count - 1 - GPU1] = t;
        // signal number of slaves for GPU1
      }

      // root broadcasts send/recv reduce plan
      MPI_Send(p, 8, MPI_UNSIGNED_CHAR, GPU1, tag, CW);

      for (i = 2; i < proc_count; i++) {
        MPI_Send((void *)&dr[i], 1, MPI_UNSIGNED_CHAR, i, tag, CW);
      }
    }
    else {
      rct[0] = rank;
      rct[1] = bi;

      MPI_Send(rct, 2, MPI_UNSIGNED, ROOT, tag, CW);
      // send number of generated boards to root

      if (rank == GPU1) {
        MPI_Recv(p, 8, MPI_UNSIGNED_CHAR, ROOT, tag, CW, &s);
        // get partition rules

        dr[0] = p[proc_count - 1 - GPU1];
        t = p[proc_count - 1 - GPU1];
        // partition split cutoff (or no-more-valid-moves signal)
      }
      else {
        MPI_Recv(dr, 1, MPI_UNSIGNED_CHAR, ROOT, tag, CW, &s);
        // wait for instruction whom to send generated boards
      }
    }

    if (dr[0] > proc_count) {
      // There are no more legal moves for any player.
      if (rank == ROOT) {
        //printf("[*] no more legal moves #2\n");
        //fflush(stdout);
      }

      re_computer_move(board, player1);
      // shallow tree, let ROOT handle it

      continue;
    }

    if (rank == GPU0) {
      for (i=0; i<proc_count; i++) {
        printf("%d ", rc[i]);
      }
      printf("\n");
      fflush(stdout);

      MPI_Send(rc, 8, MPI_UNSIGNED, GPU1, tag, CW);
      // send boards generated by each slave to GPU1

      // collect boards from other slaves
      off = bi * 8;
      for (i = 0; i<t; i++) {
        if (rc[p[i]] > 0) {
          MPI_Recv(pb + off, rc[p[i]] * 8, MPI_UNSIGNED, p[i], tag, CW, &s);
          off = off + rc[p[i]] * 8;
        }
      }

      par_mm2(pb, off >> 3, score, ctx, cq, prog, kernel);
      // query GPU0 to get scores

      // gpu workers aysnc send their scores (in a loop) to respected workers
      off = bi;
      for (i = 0; i<t; i++) {
        if (rc[p[i]] > 0) {
          MPI_Send(score + off, rc[p[i]], MPI_INT, p[i], tag, CW);
          off += rc[p[i]];
        }
      }
    }
    else if (rank == GPU1) {
      MPI_Recv(rc, 8, MPI_UNSIGNED, ROOT, tag, CW, &s);
      // get number of boards generated by each slave from ROOT

      // collect boards from other slaves
      off = bi * 8;
      for (i = t; i<proc_count - 2; i++) {
        if (rc[p[i]] > 0) {
          MPI_Recv(pb + off, rc[p[i]] * 8, MPI_UNSIGNED, p[i], tag, CW, &s);
          off = off + rc[p[i]] * 8;
        }
      }

      par_mm2(pb, off >> 3, score, ctx, cq, prog, kernel);
      // query GPU1 to get scores

      // gpu workers aysnc send their scores (in a loop) to respected workers
      off = bi;
      for (i = t; i<proc_count - 2; i++) {
        if (rc[p[i]] > 0) {
          MPI_Send(score + off, rc[p[i]], MPI_INT, p[i], tag, CW);
          off += rc[p[i]];
        }
      }
    }
    else {
      if (bi > 0) {
        MPI_Send(pb, bi * 8, MPI_UNSIGNED, dr[0], tag, CW);
        // workers send their boards to GPU0 or GPU1 based on partition plan

        MPI_Recv(score, bi * 8, MPI_INT, dr[0], tag, CW, &s);
        // wait for scores to get from assigned GPU
      }
    }

    if (bi > 0) {
      re_computer_move2(board, player1, rank, score, rcs);
      // re-traverse game tree
    }

    if (rank == ROOT) {
      off = bi;
      row = rcs[0]; col = rcs[1]; best_score = rcs[2];

      // get scores from all slaves
      for (i = 1; i < proc_count; i++) {
        if (rc[i] > 0) {
          MPI_Recv(rcs, 3, MPI_INT, i, MPI_ANY_TAG, CW, &s);
          // receive row, col and score from slaves

          if (rcs[2] * player1 > best_score * player1) {
            row = rcs[0]; col = rcs[1]; best_score = rcs[2];
          }
        }
      }

      // pick the best score
      if (row >= 0 && col >= 0) {
        //printf("(%d, %d), score: %d\n", row, col, best_score);
        //fflush(stdout);
        place_piece(row, col, player1, board);
        ctotal++;
      }
      else {
        printf("CANNOT HAPPEN (%d, %d), score: %d\n", row, col, best_score);
        fflush(stdout);
      }
    }
    else if (bi > 0) {
      MPI_Send(rcs, 3, MPI_INT, ROOT, tag, CW);
      // send row, col, score of the best score to ROOT
    }

    MPI_Barrier(CW);
  }

  t_end = MPI_Wtime();
  if (rank == ROOT) {
    printf("Time: %lf\n", t_end - t_start);

    //Izpisemo rezultat
    printf("-----GAME OVER-----\n");
    print_board(board);
    int whites = evaluate_board(board);
    printf("SCORE: Player X:%zu Player O:%d\n", ctotal - whites, whites);
  }

  if (rank == GPU0 || rank == GPU1) {
    r = clFlush(cq);
    r = clFinish(cq);
    r = clReleaseKernel(kernel);
    r = clReleaseProgram(prog);
    r = clReleaseCommandQueue(cq);
    r = clReleaseContext(ctx);
  }

  MPI_Finalize();
  return 0;
}
