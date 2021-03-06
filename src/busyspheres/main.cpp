/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2002 <hk@dgmr.nl>
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

#include "main.h"

#include <math.h>
#include <kodi/tools/Time.h>
#include <kodi/gui/gl/Texture.h>
#include <rsMath/rsMath.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define LIGHTSIZE 64

bool CScreensaverBusySpheres::Start()
{
  kodi::CheckSettingInt("general.points", m_pointsCnt);
  kodi::CheckSettingInt("general.zoom", m_zoom);

  std::string fraqShader = kodi::GetAddonPath("resources/shaders/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  for (int i = 0; i < m_pointsCnt; i++)
  {
    m_points[i][0] = rsRandf(M_PI*2.0f);
    m_points[i][1] = rsRandf(M_PI) - 0.5 * M_PI;
    m_points[i][2] = m_points[i][0];
    m_points[i][3] = m_points[i][1];
  }

  glDisable(GL_DEPTH_TEST);

  gli::texture Texture(gli::TARGET_2D, gli::FORMAT_L8_UNORM_PACK8, gli::texture::extent_type(LIGHTSIZE, LIGHTSIZE, 1), 1, 1, 1);
  int ptr = 0;
  float x, y;
  for (int i = 0; i < LIGHTSIZE; i++)
  {
    for (int j = 0; j < LIGHTSIZE; j++)
    {
      x = float(i - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
      y = float(j - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
      float temp = 1.0f - float(sqrt((x * x) + (y * y)));
      if (temp > 1.0f)
        temp = 1.0f;
      if (temp < 0.0f)
        temp = 0.0f;
      ((unsigned char*)Texture.data())[ptr++] = (unsigned char)(255.0f * temp * temp);
    }
  }
  m_texture_id = kodi::gui::gl::Load(Texture);

  glGenBuffers(1, &m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);

  glClearColor(0, 0, 0, 0);
  m_projMat = glm::mat4(1.0f);
  m_modelMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0 / Width(), 1.0 / Height(), 1.0));
  m_entryMat = glm::lookAt(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

  m_startOK = true;
  return true;
}

void CScreensaverBusySpheres::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glDeleteTextures(1, &m_texture_id);
  m_texture_id = 0;
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;
}

void CScreensaverBusySpheres::Render()
{
  float x, y, z, w, ws;

  if (!m_startOK)
    return;

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);

  glVertexAttribPointer(m_aPosition,  4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_aPosition);

  glVertexAttribPointer(m_aColor,  4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_aColor);

  glVertexAttribPointer(m_aCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_aCoord);

  double currentTime = kodi::time::GetTimeSec<double>();

  currentTime = currentTime - (int)currentTime + (int)currentTime % 86400;

  float t = 5 * currentTime + 0.35 * (cos(currentTime * 0.41 + 0.123) +
                                      cos(currentTime * 0.51 + 0.234) +
                                      cos(currentTime * 0.61 + 0.623) +
                                      cos(currentTime * 0.21 + 0.723));
  float t2 = 0.3 * currentTime;

  if ((currentTime - m_convertTime) > 10)
  {
    m_oldMode = m_newMode;
    m_newMode = rsRandi(5);
    m_convertTime = currentTime;
  }

  CalcPoints(currentTime);

  glClear(GL_COLOR_BUFFER_BIT);

  float st2 = sin(t2);
  float ct2 = cos(t2);

  // Generate the points
  for (int i = 0; i < m_pointsCnt; i++)
  {
    x = sin (m_points[i][0]) * cos (m_points[i][1]);
    y = cos (m_points[i][0]) * cos (m_points[i][1]);
    z = sin (m_points[i][1]);

    m_fb_buffer[i*37+0] = glm::vec3(x, y, z);

    x = 0.5 * x;
    y = 0.5 * y;
    z = 0.5 * z;

    m_fb_buffer[i*37+1] = glm::vec3(x + 1.5, y, z);
    m_fb_buffer[i*37+2] = glm::vec3(x - 1.5, y, z);
    m_fb_buffer[i*37+3] = glm::vec3(x, y + 1.5, z);
    m_fb_buffer[i*37+4] = glm::vec3(x, y - 1.5, z);
    m_fb_buffer[i*37+5] = glm::vec3(x, y, z + 1.5);
    m_fb_buffer[i*37+6] = glm::vec3(x, y, z - 1.5);

    x = 0.5 * x;
    y = 0.5 * y;
    z = 0.5 * z;

    m_fb_buffer[i*37+7] = glm::vec3(x + 2.25, y, z);
    m_fb_buffer[i*37+8] = glm::vec3(x + 1.5, y + 0.75 * st2, z - 0.75 * ct2);
    m_fb_buffer[i*37+9] = glm::vec3(x + 1.5, y - 0.75 * ct2, z - 0.75 * st2);
    m_fb_buffer[i*37+10] = glm::vec3(x + 1.5, y - 0.75 * st2, z + 0.75 * ct2);
    m_fb_buffer[i*37+11] = glm::vec3(x + 1.5, y + 0.75 * ct2, z + 0.75 * st2);

    m_fb_buffer[i*37+12] = glm::vec3(x - 2.25, y, z);
    m_fb_buffer[i*37+13] = glm::vec3(x - 1.5, y - 0.75 * st2, z - 0.75 * ct2);
    m_fb_buffer[i*37+14] = glm::vec3(x - 1.5, y + 0.75 * ct2, z - 0.75 * st2);
    m_fb_buffer[i*37+15] = glm::vec3(x - 1.5, y + 0.75 * st2, z + 0.75 * ct2);
    m_fb_buffer[i*37+16] = glm::vec3(x - 1.5, y - 0.75 * ct2, z + 0.75 * st2);

    m_fb_buffer[i*37+17] = glm::vec3(x, y + 2.25, z);
    m_fb_buffer[i*37+18] = glm::vec3(x + 0.75 * st2, y + 1.5, z - 0.75 * ct2);
    m_fb_buffer[i*37+19] = glm::vec3(x - 0.75 * ct2, y + 1.5, z - 0.75 * st2);
    m_fb_buffer[i*37+20] = glm::vec3(x - 0.75 * st2, y + 1.5, z + 0.75 * ct2);
    m_fb_buffer[i*37+21] = glm::vec3(x + 0.75 * ct2, y + 1.5, z + 0.75 * st2);

    m_fb_buffer[i*37+22] = glm::vec3(x, y - 2.25, z);
    m_fb_buffer[i*37+23] = glm::vec3(x - 0.75 * st2, y - 1.5, z - 0.75 * ct2);
    m_fb_buffer[i*37+24] = glm::vec3(x + 0.75 * ct2, y - 1.5, z - 0.75 * st2);
    m_fb_buffer[i*37+25] = glm::vec3(x + 0.75 * st2, y - 1.5, z + 0.75 * ct2);
    m_fb_buffer[i*37+26] = glm::vec3(x - 0.75 * ct2, y - 1.5, z + 0.75 * st2);

    m_fb_buffer[i*37+27] = glm::vec3(x, y, z + 2.25);
    m_fb_buffer[i*37+28] = glm::vec3(x + 0.75 * st2, y - 0.75 * ct2, z + 1.5);
    m_fb_buffer[i*37+29] = glm::vec3(x - 0.75 * ct2, y - 0.75 * st2, z + 1.5);
    m_fb_buffer[i*37+30] = glm::vec3(x - 0.75 * st2, y + 0.75 * ct2, z + 1.5);
    m_fb_buffer[i*37+31] = glm::vec3(x + 0.75 * ct2, y + 0.75 * st2, z + 1.5);

    m_fb_buffer[i*37+32] = glm::vec3(x, y, z - 2.25);
    m_fb_buffer[i*37+33] = glm::vec3(x - 0.75 * st2, y - 0.75 * ct2, z - 1.5);
    m_fb_buffer[i*37+34] = glm::vec3(x + 0.75 * ct2, y - 0.75 * st2, z - 1.5);
    m_fb_buffer[i*37+35] = glm::vec3(x + 0.75 * st2, y + 0.75 * ct2, z - 1.5);
    m_fb_buffer[i*37+36] = glm::vec3(x - 0.75 * ct2, y + 0.75 * st2, z - 1.5);
  }

  glm::mat4 entryMat;
  entryMat = glm::rotate(m_entryMat, glm::radians(2.4f * t), glm::vec3(1.0f, 0.0f, 0.0f));
  entryMat = glm::rotate(entryMat, glm::radians(2.5f * t), glm::vec3(0.0f, 1.0f, 0.0f));
  entryMat = glm::rotate(entryMat, glm::radians(2.6f * t), glm::vec3(0.0f, 0.0f, 1.0f));
  entryMat = glm::scale(entryMat, glm::vec3(float(m_zoom) / Width(), float(m_zoom) / Width(), 1.0));

  EnableShader();
  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_BLEND);

  // Draw objects from back to front
  sLight lights[4];
  for (int i = 0, k = 0; i < m_pointsCnt * 37; i++, k++)
  {
    if (k == 37)
      k = 0;

    glm::vec3 entry = m_fb_buffer[i] * glm::transpose(glm::inverse(glm::mat3(entryMat)));
    x = entry.x;
    y = entry.y;
    z = entry.z;

    w = 1.3 - z;  // diminishing fading
    ws = 0.002 * Height() * (1 - z);  // Keep Bitmaps same size realive to screensize

    if (k == 0)  // big sphere
    {
      lights[0].color = lights[1].color = lights[2].color = lights[3].color = sColor(0.6 * w, 0.5 * w, 0.3 * w);
      w = 70 * ws;
    }
    else if (k < 7)
    {
      lights[0].color = lights[1].color = lights[2].color = lights[3].color = sColor(0.3 * w, 0.6 * w, 0.4 * w);
      w = 30 * ws;
    }
    else
    {
      lights[0].color = lights[1].color = lights[2].color = lights[3].color = sColor(0.2 * w, 0.3 * w, 0.4 * w);
      w = 12 * ws;
    }

    lights[0].coord = sCoord(0.0, 0.0);
    lights[0].vertex = sPosition(x - w, y - w, z);

    lights[1].coord = sCoord(1.0, 0.0);
    lights[1].vertex = sPosition(x + w, y - w, z);

    lights[2].coord = sCoord(1.0, 1.0);
    lights[2].vertex = sPosition(x + w, y + w, z);

    lights[3].coord = sCoord(0.0, 1.0);
    lights[3].vertex = sPosition(x - w, y + w, z);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*4, m_idx, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, lights, GL_DYNAMIC_DRAW);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
  }

  DisableShader();

  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_BLEND);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDisableVertexAttribArray(m_aPosition);
  glDisableVertexAttribArray(m_aColor);
  glDisableVertexAttribArray(m_aCoord);
}

void CScreensaverBusySpheres::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");

  m_aPosition = glGetAttribLocation(ProgramHandle(), "a_position");
  m_aColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_aCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverBusySpheres::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));

  return true;
}

void CScreensaverBusySpheres::CalcPoints(float currentTime)
{
  int i;
  float x1, x2, y1, y2;
  float dt = currentTime - m_convertTime;

  if (dt < 5)
  {
    for (i = 0; i < m_pointsCnt; i++)
    {
      x1 = 0;
      y1 = 0;
      x2 = 0;
      y2 = 0;

      switch (m_newMode)
      {
      case EFFECT_RANDOM:
        x1 = m_points[i][2];
        y1 = m_points[i][3];

        break;

      case EFFECT_CRESCENT:
        x1 = (i * 0.2 * (1 + sin (0.3 * currentTime))) / (M_PI*2.0f);
        x1 = M_PI*2.0f * (x1 - (int)x1);
        y1 = M_PI * ((float)i / m_pointsCnt - 0.5);

        break;

      case EFFECT_DOT:
        x1 = m_points[i][2];
        y1 = ((m_points[i][3] > 0) ? 0.5 : -0.5) * M_PI - m_points[i][3] * 0.5 * (1 + sin (0.3 * currentTime));

        break;

      case EFFECT_RING:
        x1 = m_points[i][2];
        y1 = m_points[i][3] * 0.5 * (1 + sin (0.3 * currentTime));

        break;

      case EFFECT_LONGITUDE:
        x1 = i / 10.0f;
        x1 = M_PI*2.0f * (x1 - (int)x1);
        y1 = 0.9 * M_PI * ((float)i / m_pointsCnt - 0.5);
      }

      switch (m_oldMode)
      {
      case EFFECT_RANDOM:
        x2 = m_points[i][2];
        y2 = m_points[i][3];

        break;

      case EFFECT_CRESCENT:
        x2 = (i * 0.2 * (1 + sin (0.3 * currentTime))) / (M_PI*2.0f);
        x2 = M_PI*2.0f * (x2 - (int)x2);
        y2 = M_PI * ((float)i / m_pointsCnt - 0.5);

        break;

      case EFFECT_DOT:
        x2 = m_points[i][2];
        if (m_points[i][3] > 0) {
          y2 = 0.5 * M_PI - m_points[i][3] * 0.5 * (1 + sin (0.3 * currentTime));
        } else {
          y2 = -0.5 * M_PI - m_points[i][3] * 0.5 * (1 + sin (0.3 * currentTime));
        }

        break;

      case EFFECT_RING:
        x2 = m_points[i][2];
        y2 = m_points[i][3] * 0.5 * (1 + sin (0.3 * currentTime));

        break;

      case EFFECT_LONGITUDE:
        x2 = i / 10.0f;
        x2 = M_PI*2.0f * (x1 - (int)x2);
        y2 = 0.9 * M_PI * ((float)i / m_pointsCnt - 0.5);
      }

      m_points[i][0] = 0.2 * (x1 * dt + x2 * (5 - dt));
      m_points[i][1] = 0.2 * (y1 * dt + y2 * (5 - dt));
    }
  }
  else
  {
    switch (m_newMode)
    {
    case EFFECT_RANDOM:
      for (i = 0; i < m_pointsCnt; i++)
      {
        m_points[i][0] = m_points[i][2];
        m_points[i][1] = m_points[i][3];
      }

      break;

    case EFFECT_CRESCENT:
      for (i = 0; i < m_pointsCnt; i++)
      {
        m_points[i][0] = (i * 0.2 * (1 + sin (0.3 * currentTime))) / (M_PI*2.0f);
        m_points[i][0] = M_PI*2.0f * (m_points[i][0] - (int)m_points[i][0]);
        m_points[i][1] = M_PI * ((float)i / m_pointsCnt - 0.5);
      }

      break;

    case EFFECT_DOT:
      for (i = 0; i < m_pointsCnt; i++)
      {
        m_points[i][0] = m_points[i][2];
        m_points[i][1] = ((m_points[i][3] > 0) ? 0.5 : -0.5) * M_PI - m_points[i][3] * 0.5 * (1 + sin (0.3 * currentTime));
      }

      break;

    case EFFECT_RING:
      for (i = 0; i < m_pointsCnt; i++)
      {
        m_points[i][0] = m_points[i][2];
        m_points[i][1] = m_points[i][3] * 0.5 * (1 + sin (0.3 * currentTime));
      }

      break;

    case EFFECT_LONGITUDE:
      for (i = 0; i < m_pointsCnt; i++)
      {
        m_points[i][0] = i / 10.0f;
        m_points[i][0] = M_PI*2.0f * (m_points[i][0] - (int)m_points[i][0]);
        m_points[i][1] = 0.9 * M_PI * ((float)i / m_pointsCnt - 0.5);
      }
    }
  }
}

ADDONCREATOR(CScreensaverBusySpheres);
