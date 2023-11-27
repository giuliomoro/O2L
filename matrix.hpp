
/*--------------------------------------------------------------------
  This file is part of the Adafruit NeoMatrix library.

  NeoMatrix is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoMatrix is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoMatrix.  If not, see
  <http://www.gnu.org/licenses/>.
  --------------------------------------------------------------------*/

#ifndef _MATRIX_HPP_
#define _MATRIX_HPP_

#include "gfx.hpp"
#include "pixel.hpp"

// Matrix layout information is passed in the 'matrixType' parameter for
// each constructor (the parameter immediately following is the LED type
// from NeoPixel.h).

// These define the layout for a single 'unified' matrix (e.g. one made
// from NeoPixel strips, or a single NeoPixel shield), or for the pixels
// within each matrix of a tiled display (e.g. multiple NeoPixel shields).

#define MATRIX_TOP 0x00         // Pixel 0 is at top of matrix
#define MATRIX_BOTTOM 0x01      // Pixel 0 is at bottom of matrix
#define MATRIX_LEFT 0x00        // Pixel 0 is at left of matrix
#define MATRIX_RIGHT 0x02       // Pixel 0 is at right of matrix
#define MATRIX_CORNER 0x03      // Bitmask for pixel 0 matrix corner
#define MATRIX_ROWS 0x00        // Matrix is row major (horizontal)
#define MATRIX_COLUMNS 0x04     // Matrix is column major (vertical)
#define MATRIX_AXIS 0x04        // Bitmask for row/column layout
#define MATRIX_PROGRESSIVE 0x00 // Same pixel order across each line
#define MATRIX_ZIGZAG 0x08      // Pixel order reverses between lines
#define MATRIX_SEQUENCE 0x08    // Bitmask for pixel line order

// These apply only to tiled displays (multiple matrices):

#define TILE_TOP 0x00         // First tile is at top of matrix
#define TILE_BOTTOM 0x10      // First tile is at bottom of matrix
#define TILE_LEFT 0x00        // First tile is at left of matrix
#define TILE_RIGHT 0x20       // First tile is at right of matrix
#define TILE_CORNER 0x30      // Bitmask for first tile corner
#define TILE_ROWS 0x00        // Tiles ordered in rows
#define TILE_COLUMNS 0x40     // Tiles ordered in columns
#define TILE_AXIS 0x40        // Bitmask for tile H/V orientation
#define TILE_PROGRESSIVE 0x00 // Same tile order across each line
#define TILE_ZIGZAG 0x80      // Tile order reverses between lines
#define TILE_SEQUENCE 0x80    // Bitmask for tile line order

 class PixelBone_Matrix : public PixelBone_GFX, public PixelBone_Pixel {

public:
  // Constructor for single matrix:
  PixelBone_Matrix(int w, int h,
                   uint8_t matrixType = MATRIX_TOP + MATRIX_LEFT + MATRIX_ROWS);

  // Constructor for tiled matrices:
  PixelBone_Matrix(uint8_t matrixW, uint8_t matrixH, uint8_t tX, uint8_t tY,
                   uint8_t matrixType = MATRIX_TOP + MATRIX_LEFT + MATRIX_ROWS +
                                        TILE_TOP + TILE_LEFT + TILE_ROWS);

  void drawPixel(int16_t x, int16_t y, uint32_t color);
  uint16_t getPixelColor(int16_t x, int16_t y);
  void fillScreen(uint32_t color);
  void setRemapFunction(uint16_t (*fn)(uint16_t, uint16_t));


private:
  const uint8_t type;
  const uint8_t matrixWidth, matrixHeight, tilesX, tilesY;
  uint16_t (*remapFn)(uint16_t x, uint16_t y);
  int getOffset(int16_t x, int16_t y);
};

#endif // _MATRIX_HPP_
