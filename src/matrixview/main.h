/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2003 Alexander Zolotov, Eugene Zolotov
 *  Copyright (C) 2008-2014 Vincent Launchbury
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *  This file is part of Kodi - https://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 */

/*
 * Code is based on:
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#pragma once

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/gtc/type_ptr.hpp>

struct sPosition
{
  sPosition() : x(0.0f), y(0.0f), z(1.0f), u(1.0f) {}
  sPosition(float* d) : x(d[0]), y(d[1]), z(d[2]), u(1.0f) {}
  sPosition(float x, float y, float z = 0.0f) : x(x), y(y), z(z), u(1.0f) {}
  float x,y,z,u;
};

struct sCoord
{
  sCoord() : s(0.0f), t(0.0f) {}
  sCoord(float s, float t) : s(s), t(t) {}
  float s,t;
};

struct sColor
{
  sColor() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
  sColor(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
  sColor& operator=(float* rhs)
  {
    r = rhs[0];
    g = rhs[1];
    b = rhs[2];
    return *this;
  }
  float r,g,b,a;
};

struct sLight
{
  sPosition vertex;
  sPosition normal;
  sColor color;
  sCoord coord;
};

typedef enum
{
  MATRIX_COLOR_RED = 0,
  MATRIX_COLOR_GREEN = 1,
  MATRIX_COLOR_BLUE = 2
} MATRIX_COLOR;

class ATTRIBUTE_HIDDEN CScreensaverMatrixView
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverMatrixView() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  void draw_char(long num, float light, float x, float y, float z);
  void draw_flare(float x,float y,float z);
  void draw_text1();
  void draw_text2(int mode);
  void scroll();
  void make_change();

  int m_pic_offset;      /* Which image to show */

  struct glyph {
    unsigned char num;   /* Character number (0-59) */
    unsigned char alpha; /* Alpha modifier */
    float z;             /* Cached Z coordinate */
  } *m_glyphs;

  /* Global Variables */
  #define rtext_x 90
  #define text_y 70
  int m_text_x;
  unsigned char *m_speeds; /* Speed of each column (0-2) */

  // Settings
  //@{
  int m_color = MATRIX_COLOR_GREEN;     /* Color of text */
  int m_rain_intensity = 1;             /* Intensity of digital rain */
  long m_timer = 40;                    /* Controls pic fade in/out */
  int m_pic_fade = 0;                   /* Makes all chars lighter/darker */
  bool m_classic = true;                    /* classic mode (no 3d) */
  int m_num_pics = 11 -1;               /* # 3d images (0 indexed) */
  //@}

  glm::mat4 m_projModelMat;

  GLint m_projModelMatLoc = -1;
  GLint m_colorLoc = -1;
  GLint m_positionLoc = -1;
  GLint m_texCoord0Loc = -1;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;

  GLuint m_texture1 = 0;
  GLuint m_texture2 = 0;
  GLuint m_texture3 = 0;

  sLight m_light[4];
  GLuint m_indexer[4] = {0, 1, 3, 2};

  bool m_startOK = false;
};
