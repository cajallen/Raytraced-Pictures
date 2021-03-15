//CSCI 5607 Header-only Image Library

#ifndef IMAGE_LIB_H
#define IMAGE_LIB_H

#include "stb_image.h"
#include "stb_image_write.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


#define LERP(a, b, r) ((1.0 - r) * a + r * b)

using namespace std;

struct Color{
    float r,g,b;

    Color(float r, float g, float b) : r(r), g(g), b(b) {}
    Color() : r(0), g(0), b(0) {}

    Color Lerp(const Color& rhs, float amt) const;
    void Clamp();

    Color operator+(const Color& rhs) const;
    Color operator*(const Color& rhs) const;
    Color operator*(const float& rhs) const;
    bool operator==(const Color& rhs) const;
};

struct Image{
  int width, height;
  Color* pixels;

  Image(int w, int h);
  Image(const Image& cpy);
  Image(const char* fname);
  ~Image();

  void setPixel(int i, int j, Color c);
  Color& getPixel(int i, int j);
  uint8_t* toBytes();
  void write(const char* fname);

  Image& operator=(const Image& rhs);
};

#endif