/*-------------------------------------------------------------------------
  Arduino library to control single and tiled matrices of WS2811- and
  WS2812-based RGB LED devices such as the Adafruit NeoPixel Shield or
  displays assembled from NeoPixel strips, making them compatible with
  the PixelBone_GFX graphics library.  Requires both the Adafruit_NeoPixel
  and PixelBone_GFX libraries.

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  -------------------------------------------------------------------------
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
  -------------------------------------------------------------------------*/

#include "matrix.hpp"
#include "gamma.h"
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

// Constructor for single matrix:
PixelBone_Matrix::PixelBone_Matrix(int w, int h, uint8_t matrixType)
    : PixelBone_GFX(w, h), PixelBone_Pixel(w * h), type(matrixType),
      matrixWidth(w), matrixHeight(h), tilesX(0), tilesY(0), remapFn(NULL) {}

// Constructor for tiled matrices:
PixelBone_Matrix::PixelBone_Matrix(uint8_t mW, uint8_t mH, uint8_t tX,
                                   uint8_t tY, uint8_t matrixType)
    : PixelBone_GFX(mW * tX, mH * tY), PixelBone_Pixel(mW * mH * tX * tY),
      type(matrixType), matrixWidth(mW), matrixHeight(mH), tilesX(tX),
      tilesY(tY), remapFn(NULL) {}


int PixelBone_Matrix::getOffset(int16_t x, int16_t y) {

  if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
    return -1;

  int16_t t;
  switch (rotation) {
  case 1:
    t = x;
    x = WIDTH - 1 - y;
    y = t;
    break;
  case 2:
    x = WIDTH - 1 - x;
    y = HEIGHT - 1 - y;
    break;
  case 3:
    t = x;
    x = y;
    y = HEIGHT - 1 - t;
    break;
  }

  int tileOffset = 0, pixelOffset;

  if (remapFn) { // Custom X/Y remapping function
    pixelOffset = (*remapFn)(x, y);
  } else { // Standard single matrix or tiled matrices

    uint8_t corner = type & MATRIX_CORNER;
    uint16_t minor, major, majorScale;

    if (tilesX) { // Tiled display, multiple matrices
      uint16_t tile;

      minor = x / matrixWidth;           // Tile # X/Y; presume row major to
      major = y / matrixHeight,          // start (will swap later if needed)
          x = x - (minor * matrixWidth); // Pixel X/Y within tile
      y = y - (major * matrixHeight);    // (-* is less math than modulo)

      // Determine corner of entry, flip axes if needed
      if (type & TILE_RIGHT)
        minor = tilesX - 1 - minor;
      if (type & TILE_BOTTOM)
        major = tilesY - 1 - major;

      // Determine actual major axis of tiling
      if ((type & TILE_AXIS) == TILE_ROWS) {
        majorScale = tilesX;
      } else {
        swap(major, minor);
        majorScale = tilesY;
      }

      // Determine tile number
      if ((type & TILE_SEQUENCE) == TILE_PROGRESSIVE) {
        // All tiles in same order
        tile = major * majorScale + minor;
      } else {
        // Zigzag; alternate rows change direction.  On these rows,
        // this also flips the starting corner of the matrix for the
        // pixel math later.
        if (major & 1) {
          corner ^= MATRIX_CORNER;
          tile = (major + 1) * majorScale - 1 - minor;
        } else {
          tile = major * majorScale + minor;
        }
      }

      // Index of first pixel in tile
      tileOffset = tile * matrixWidth * matrixHeight;

    } // else no tiling (handle as single tile)

    // Find pixel number within tile
    minor = x; // Presume row major to start (will swap later if needed)
    major = y;

    // Determine corner of entry, flip axes if needed
    if (corner & MATRIX_RIGHT)
      minor = matrixWidth - 1 - minor;
    if (corner & MATRIX_BOTTOM)
      major = matrixHeight - 1 - major;

    // Determine actual major axis of matrix
    if ((type & MATRIX_AXIS) == MATRIX_ROWS) {
      majorScale = matrixWidth;
    } else {
      swap(major, minor);
      majorScale = matrixHeight;
    }

    // Determine pixel number within tile/matrix
    if ((type & MATRIX_SEQUENCE) == MATRIX_PROGRESSIVE) {
      // All lines in same order
      pixelOffset = major * majorScale + minor;
    } else {
      // Zigzag; alternate rows change direction.
      if (major & 1)
        pixelOffset = (major + 1) * majorScale - 1 - minor;
      else
        pixelOffset = major * majorScale + minor;
    }
  }
  int offset = (tileOffset + pixelOffset);
  return offset;
}

void PixelBone_Matrix::drawPixel(int16_t x, int16_t y, uint32_t color) {
  setPixelColor(getOffset(x,y), color);
}

uint16_t PixelBone_Matrix::getPixelColor(int16_t x, int16_t y) {
  // uint32_t color = expandColor(PixelBone_Pixel::getPixelColor(getOffset(x,y)));
  // uint8_t r = (uint8_t) (color & 0xFF);         // first 8 bits
  // uint8_t g = (uint8_t) ((color >> 8) & 0xFF);  // second 8 bits
  // uint8_t b = (uint8_t) ((color >> 16) & 0xFF); // third 8 bits 

  pixel_t *const p = PixelBone_Pixel::getPixel(getOffset(x,y));
  // printf("Red color: 0x%x\n", p->r);
  // printf("Green color: 0x%x\n", p->g);
  // printf("Blue color: 0x%x\n", p->b);

  return PixelBone_Matrix::Color(p->r, p->g, p->b);
}

void PixelBone_Matrix::fillScreen(uint32_t color) {
  uint32_t n = numPixels();

  for (uint32_t i = 0; i < n; i++)
    setPixelColor(i, color);
}

void PixelBone_Matrix::setRemapFunction(uint16_t (*fn)(uint16_t, uint16_t)) {
  remapFn = fn;
}
