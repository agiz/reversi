//Definiramo velikost igralnega polja
#define BOARD_HEIGHT 8
#define BOARD_WIDTH 8

#define W 0
#define B 8
// keep track of two separate boards, one for white, one for black
// first 8 bytes (64 bits) is for white

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((y) > (x)) ? (x) : (y))

#define SET_MIN_MAX(x, y, min, max) (((x) > (y)) ? ({ max = x; min = y; }) : ({ max = y; min = x; }))

#define PACK2(a, player, x, y, o) \
  a[(o) + (player << 3) + (x)] |= (1 << (7 - (y))); \
  a[(o) + ((1 - player ) << 3) + (x)] &= ~(1 << (7 - (y))) & 0xff; \
  a[(o) + ((player + 2) << 3) + (y)] |= (1 << (7 - (x))); \
  a[(o) + ((1 - player + 2) << 3) + (y)] &= ~(1 << (7 - (x))) & 0xff;

#define CPACK2(a, player, s, x, y, o) \
  a[(o) + ((player + ((s) << 1)) << 3) + (x)] |= (1 << (7 - (y))); \
  a[(o) + ((1 - player + ((s) << 1)) << 3) + (x)] &= ~(1 << (7 - (y))) & 0xff; \
  a[(o) + ((player + 2 - ((s) << 1)) << 3) + (y)] |= (1 << (7 - (x))); \
  a[(o) + ((1 - player + 2 - ((s) << 1)) << 3) + (y)] &= ~(1 << (7 - (x))) & 0xff;

#define TILE(a, player, x, y) ((a[((player << 3) + (x))] >> (7 - (y))) & 0x1)

#define COPY_BOARD(boards, board, i, j) \
  boards[((i) << 5) + 0] = board[((j) << 5) + 0]; \
  boards[((i) << 5) + 1] = board[((j) << 5) + 1]; \
  boards[((i) << 5) + 2] = board[((j) << 5) + 2]; \
  boards[((i) << 5) + 3] = board[((j) << 5) + 3]; \
  boards[((i) << 5) + 4] = board[((j) << 5) + 4]; \
  boards[((i) << 5) + 5] = board[((j) << 5) + 5]; \
  boards[((i) << 5) + 6] = board[((j) << 5) + 6]; \
  boards[((i) << 5) + 7] = board[((j) << 5) + 7]; \
  boards[((i) << 5) + 8] = board[((j) << 5) + 8]; \
  boards[((i) << 5) + 9] = board[((j) << 5) + 9]; \
  boards[((i) << 5) + 10] = board[((j) << 5) + 10]; \
  boards[((i) << 5) + 11] = board[((j) << 5) + 11]; \
  boards[((i) << 5) + 12] = board[((j) << 5) + 12]; \
  boards[((i) << 5) + 13] = board[((j) << 5) + 13]; \
  boards[((i) << 5) + 14] = board[((j) << 5) + 14]; \
  boards[((i) << 5) + 15] = board[((j) << 5) + 15]; \
  boards[((i) << 5) + 16] = board[((j) << 5) + 16]; \
  boards[((i) << 5) + 17] = board[((j) << 5) + 17]; \
  boards[((i) << 5) + 18] = board[((j) << 5) + 18]; \
  boards[((i) << 5) + 19] = board[((j) << 5) + 19]; \
  boards[((i) << 5) + 20] = board[((j) << 5) + 20]; \
  boards[((i) << 5) + 21] = board[((j) << 5) + 21]; \
  boards[((i) << 5) + 22] = board[((j) << 5) + 22]; \
  boards[((i) << 5) + 23] = board[((j) << 5) + 23]; \
  boards[((i) << 5) + 24] = board[((j) << 5) + 24]; \
  boards[((i) << 5) + 25] = board[((j) << 5) + 25]; \
  boards[((i) << 5) + 26] = board[((j) << 5) + 26]; \
  boards[((i) << 5) + 27] = board[((j) << 5) + 27]; \
  boards[((i) << 5) + 28] = board[((j) << 5) + 28]; \
  boards[((i) << 5) + 29] = board[((j) << 5) + 29]; \
  boards[((i) << 5) + 30] = board[((j) << 5) + 30]; \
  boards[((i) << 5) + 31] = board[((j) << 5) + 31];

#define MAX_H 1

//Podatkovne strukture potrebne pri postavljanju novega ploscka

__constant unsigned char n_of_bits[256] = {
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

__constant unsigned char ffs_lu[32] = {
  0, 1, 2, 1, 3, 1, 2, 1,
  4, 1, 2, 1, 3, 1, 2, 1,
  5, 1, 2, 1, 3, 1, 2, 1,
  4, 1, 2, 1, 3, 1, 2, 1
};

__constant unsigned char clz_lu[32] = {
  8, 7, 6, 6, 5, 5, 5, 5,
  4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3
};

int is_move_valid(char player, unsigned char *board, char row, char col, unsigned int off) {
  if (((board[off + row] | board[off + B + row]) >> (7 - col)) & 0x1) {
    return 0;
  }

  player = 1 - ((1 + player) >> 1);
  unsigned char i, d, f, s, p, t, v, tmp;
  char u;
  char cont = 0;
  unsigned char j, dd, l;

  s = board[off + ((player + (0 << 1)) << 3) + row];
  // """row of players discs"""

  p = board[off + ((((1 - player) + (0 << 1))) << 3) + row];
  // """row of opponents discs"""

  if (col < 6 && ((p >> (6 - col)) & 0x1)) {

    t = s & (0x7f >> (1 + col));

    v = (t >> 5) & 0x1;
    // """MSB"""

    u = clz_lu[t & 0x1f];
    u = 7 - ((u & (~(-v) & 0xf)) | (v << 1));

    if (u > -1 && (((p | s) >> u) & (0x7f >> (col + u))) == (0x7f >> (col + u))) {
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
      cont = 1;
      //continue;
    }

    if (!cont && ((((p | s) >> (9 - col)) & (0xff >> (8 - u))) == (0xff >> (8 - u)))) {
      return 1;
    }
  }

  if (!cont) {
    tmp = row;
    row = col;
    col = tmp;
  }

  cont = 0;

  s = board[off + ((player + (1 << 1)) << 3) + row];
  // """row of players discs"""

  p = board[off + ((((1 - player) + (1 << 1))) << 3) + row];
  // """row of opponents discs"""

  if (col < 6 && ((p >> (6 - col)) & 0x1)) {

    t = s & (0x7f >> (1 + col));

    v = (t >> 5) & 0x1;
    // """MSB"""

    u = clz_lu[t & 0x1f];
    u = 7 - ((u & (~(-v) & 0xf)) | (v << 1));

    if (u > -1 && (((p | s) >> u) & (0x7f >> (col + u))) == (0x7f >> (col + u))) {
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
      cont = 1;
      //continue;
    }

    if (!cont && ((((p | s) >> (9 - col)) & (0xff >> (8 - u))) == (0xff >> (8 - u)))) {
      return 1;
    }
  }

  if (!cont) {
    tmp = row;
    row = col;
    col = tmp;
  }

  cont = 1;

  SET_MIN_MAX(row, col, f, d);

  i = 1;
  // # pristevamo 1 po diagonali
  if (d < 6) {
    v = TILE(board, (1 - player + 2 - (i << 1)), off + row + 1, col + 1);

    if (v) {
      j = 2;
      dd = 8 - d;
      if (j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0) { cont = 0; }
      }


    }
  }

  cont = 1;

  if (f > 1) {
    v = TILE(board, (1 - player + 2 - (i << 1)), off + row - 1, col - 1);

    if (v) {
      j = 2;
      dd = f + 1;
      if (j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0) { cont = 0; }
      }

    }
  }

  i = 1;
  cont = 1;

  //print 'kontra diagonala 1'
  if (col < 6 && row > 1) {
    v = TILE(board, (1 - player + 2 - (i << 1)), off + row - 1, col + 1);

    l = MIN(row + col, 7) - col + 1;

    if (v) {
      j = 2;
      dd = l;
      if (j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0) { cont = 0; }
      }
    }
  }

  cont = 1;

  //print 'kontra diagonala 2'
  if (col > 1 && row < 6) {
    v = TILE(board, (1 - player + 2 - (i << 1)), off + row + 1, col - 1);

    l = MIN(row + col, 7) - row + 1;

    if (v) {
      j = 2;
      dd = l;
      if (j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { return 1; }
        if (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0) { cont = 0; }
      }
    }
  }

  // print 'not found'
  return 0;
}

void place_piece2(int row, int col, char player, unsigned char *board, unsigned int off) {
  player = 1 - ((1 + player) >> 1);

  unsigned char c, j, l, takeover, dd, cont;
  unsigned char i, d, f, s, p, t, v, tmp;
  char u;

  cont = 0;
  i = 0;

  s = board[off + ((player + (i << 1)) << 3) + row];
  // """row of players discs"""

  p = board[off + (((1 - player + (i << 1))) << 3) + row];
  // """row of opponents discs"""

  // looking right
  if (col < 6 && ((p >> (6 - col)) & 0x1)) {

    t = s & (0x7f >> (1 + col));

    v = (t >> 5) & 0x1;
    // """MSB"""

    u = clz_lu[t & 0x1f];
    u = 7 - ((u & (~(-v) & 0xf)) | (v << 1));

    if (u > -1 && (((p | s) >> u) & (0x7f >> (col + u))) == (0x7f >> (col + u))) {
      // barvanje
      dd = 7 - u;
      c = col;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
    }
  }

  // looking left
  if ((col > 1 && ((p >> (8 - col)) & 0x1))) {
    t = s >> (9 - col);
    u = ffs_lu[t & 0x1f];

    // TODO: just like above, in one line.
    if (u == 0 && t == 32) {
      u = 6;
    }
    else if (u == 0) {
      tmp = row;
      row = col;
      col = tmp;
      cont = 1;
      //continue;
    }

    if (!cont && ((((p | s) >> (9 - col)) & (0xff >> (8 - u))) == (0xff >> (8 - u)))) {
      // barvanje
      dd = col + 1;
      c = col - u;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
    }
  }

  if (!cont) {
    tmp = row;
    row = col;
    col = tmp;
  }

  cont = 0;
  i = 1;
  //
  s = board[off + ((player + (i << 1)) << 3) + row];
  // """row of players discs"""

  p = board[off + (((1 - player + (i << 1))) << 3) + row];
  // """row of opponents discs"""

  // looking right
  if (col < 6 && ((p >> (6 - col)) & 0x1)) {

    t = s & (0x7f >> (1 + col));

    v = (t >> 5) & 0x1;
    // """MSB"""

    u = clz_lu[t & 0x1f];
    u = 7 - ((u & (~(-v) & 0xf)) | (v << 1));

    if (u > -1 && (((p | s) >> u) & (0x7f >> (col + u))) == (0x7f >> (col + u))) {
      // barvanje
      dd = 7 - u;
      c = col;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
    }
  }

  // looking left
  if ((col > 1 && ((p >> (8 - col)) & 0x1))) {
    t = s >> (9 - col);
    u = ffs_lu[t & 0x1f];

    // TODO: just like above, in one line.
    if (u == 0 && t == 32) {
      u = 6;
    }
    else if (u == 0) {
      tmp = row;
      row = col;
      col = tmp;
      cont = 1;
      //continue;
    }

    if (!cont && ((((p | s) >> (9 - col)) & (0xff >> (8 - u))) == (0xff >> (8 - u)))) {
      // barvanje
      dd = col + 1;
      c = col - u;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
      c++;
      if (c < dd) {
        CPACK2(board, player, i, row, c, off);
      }
    }
  }

  if (!cont) {
    tmp = row;
    row = col;
    col = tmp;
  }

  SET_MIN_MAX(row, col, f, d);

  cont = 1;
  i = 1;
  // # pristevamo 1 po diagonali
  if (d < 6) {
    takeover = 0;
    v = TILE(board, (1 - player + 2 - (i << 1)), off + row + 1, col + 1);

    if (v) {

      dd = 8 - d;
      j = 2;
      if (j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col + j) == 0)) { cont = 0; }
      }


      // barvanje
      if (takeover) {

        dd = takeover;
        c = 0;
        if (c < dd) {
          PACK2(board, player, row + c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col + c, off);
        }
      }
    }
  }

  cont = 1;
  if (f > 1) {
    takeover = 0;
    v = TILE(board, (1 - player + 2 - (i << 1)), off + row - 1, col - 1);

    if (v) {

      dd = f + 1;
      j = 2;
      if (j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col - j) == 0)) { cont = 0; }
      }


      // barvanje
      if (takeover) {

        dd = takeover;
        c = 0;
        if (c < dd) {
          PACK2(board, player, row - c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col - c, off);
        }
      }
    }
  }

  cont = 1;
  i = 1;

  //print 'kontra diagonala 1'
  if (col < 6 && row > 1) {
    takeover = 0;
    v = TILE(board, (1 - player + 2 - (i << 1)), off + row - 1, col + 1);

    l = MIN(row + col, 7) - col + 1;

    if (v) {

      dd = l;
      j = 2;
      if (j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row - j, col + j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row - j, col + j) == 0)) { cont = 0; }
      }

      // barvanje
      if (takeover) {
        dd = takeover;
        c = 0;
        if (c < dd) {
          PACK2(board, player, row - c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col + c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row - c, col + c, off);
        }
      }
    }
  }

  cont = 1;

  //print 'kontra diagonala 2'
  if (col > 1 && row < 6) {
    takeover = 0;
    v = TILE(board, (1 - player + 2 - (i << 1)), off + row + 1, col - 1);

    l = MIN(row + col, 7) - row + 1;

    if (v) {

      dd = l;
      j = 2;
      if (j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0)) { cont = 0; }
      }
      j++;
      if (cont && j < dd) {
        if (TILE(board, (player + 2 - (i << 1)), off + row + j, col - j)) { takeover = j; cont = 0; }
        if (cont && (TILE(board, (1 - player + 2 - (i << 1)), off + row + j, col - j) == 0)) { cont = 0; }
      }

      // barvanje
      if (takeover) {
        dd = takeover;
        c = 0;
        if (c < dd) {
          PACK2(board, player, row + c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col - c, off);
        }
        c++;
        if (c < dd) {
          PACK2(board, player, row + c, col - c, off);
        }
      }
    }
  }
}

#define EVAL_BOARD(board, i) \
  n_of_bits[board[(i) + 0]] + n_of_bits[board[(i) + 1]] + n_of_bits[board[(i) + 2]] + n_of_bits[board[(i) + 3]] + n_of_bits[board[(i) + 4]] + n_of_bits[board[(i) + 5]] + n_of_bits[board[(i) + 6]] + n_of_bits[board[(i) + 7]];

//stack minimax
__kernel void minimax(
    __global const register unsigned char *cboard,
    __global int *out_score,
    const unsigned int board_n
    )
{
  const size_t gid = get_global_id(0);
  // blockDim.x * blockId.x + thraedId.x

  if (gid >= board_n) { return; }

  char player = -1, h = 0;
  unsigned char i, j, br = 0;
  int score[MAX_H + 2];
  score[0] = -INT_MAX * player;
  //Nastavimo najmanjse mozno zacetno stevilo tock

  // temporary stack structures
  unsigned char is[MAX_H + 2], js[MAX_H + 2], moved[MAX_H + 2];
  moved[0] = 0; is[0] = 0; js[0] = 0;

  unsigned char tmp_b[(MAX_H + 2) << 5];

  COPY_BOARD(tmp_b, cboard, 0, gid);
  // copy original board to beginning of the temp boards.

  while (1) {
    //Pogledamo ce smo ze prisli do najvecje mozne globine rekurzije
    if (h > MAX_H) {
      score[h] = EVAL_BOARD(tmp_b, ((h) << 5));

      h--;
      // reduce one level.

      player = -player;

      if (score[h + 1] * player > score[h] * player) {
        score[h] = score[h + 1];
      }
    }

    for (i = is[h]; i < BOARD_HEIGHT; i++) {
      for (j = js[h]; j < BOARD_WIDTH; j++) {

        //Preverimo ce je nasprotnikova poteza veljavna
        if (is_move_valid(-player, tmp_b, i, j, (h << 5))) {
          moved[h] = 1;
          is[h] = i; js[h] = j + 1;
          player = -player;

          COPY_BOARD(tmp_b, tmp_b, h + 1, h);
          place_piece2(i, j, player, tmp_b, (h + 1) << 5);

          h++;
          score[h] = -INT_MAX * player;
          moved[h] = 0; is[h] = 0; js[h] = 0;

          br = 1; break;
        }
      }
      if (br) { break; }
      else { js[h] = 0; }
    }

    if (!br) {
      // Ce ni bilo mozno narediti nobene poteze
      // evalviramo igralno polje in koncamo
      if (!moved[h]) { score[h] = EVAL_BOARD(tmp_b, ((h) << 5)); }

      h--;
      // take off the stack

      if (h < 0) { out_score[gid] = score[0]; return; }

      // continue, where we left...
      player = -player;

      if (score[h + 1] * player > score[h] * player) {
        score[h] = score[h + 1];
      }
    } //cont. on while

    br = 0;
  }
}
