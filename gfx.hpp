#include <stdint.h>
#ifndef _GFX_HPP_
#define _GFX_HPP_

#include <string>

#define swap(a, b)                                                             \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }

class PixelBone_GFX {

public:
  PixelBone_GFX(int16_t w, int16_t h); // Constructor

  // This MUST be defined by the subclass:
  virtual void drawPixel(int16_t x, int16_t y, uint32_t color) = 0;

  // These MAY be overridden by the subclass to provide device-specific
  // optimized code.  Otherwise 'generic' versions are used.
  virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                        uint32_t color);
  virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint32_t color);
  virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint32_t color);
  virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                        uint32_t color);
  virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                        uint32_t color);
  virtual void fillScreen(uint32_t color);
  virtual void invertDisplay(bool i);

  // These exist only with Adafruit_GFX (no subclass overrides)
  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint32_t color);
  void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
                        uint32_t color);
  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint32_t color);
  void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
                        int16_t delta, uint32_t color);
  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                    int16_t y2, uint32_t color);
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2,
                    int16_t y2, uint32_t color);
  void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
                     int16_t radius, uint32_t color);
  void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
                     int16_t radius, uint32_t color);
  void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                  int16_t h, uint32_t color);
  void drawChar(int16_t x, int16_t y, unsigned char c, uint32_t color,
                uint32_t bg, uint8_t size);
  void setCursor(int16_t x, int16_t y);
  void setTextColor(uint32_t color);
  void setTextColor(uint32_t color, uint32_t bg);
  void setTextSize(uint8_t s);
  void setTextWrap(bool w);
  void setRotation(uint8_t r);

  void print(const std::string &s);
  void print(const char str[]);
  void write(const char *str);
  void write(const uint8_t *buffer, size_t size);
  virtual void write(uint8_t);

  int16_t height(void);
  int16_t width(void);
  uint8_t getRotation(void);

protected:
  const int16_t WIDTH, HEIGHT; // This is the 'raw' display w/h - never changes
  int16_t _width, _height;     // Display w/h as modified by current rotation
  int16_t cursor_x, cursor_y;
  uint32_t textcolor, textbgcolor;
  uint8_t textsize;
  uint8_t rotation;
  bool wrap; // If set, 'wrap' text at right edge of display
};

#endif // _GFX_HPP_
