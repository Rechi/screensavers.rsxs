/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2010 Terence M. Welsh
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
 *   https://github.com/reallyslickscreensavers/reallyslickscreensavers
 * and reworked to GL 4.0.
 */

#pragma once

#include <Implicit/impSphere.h>
#include <Implicit/impCapsule.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN Tetrahedron : public Gizmo
{
public:
  Tetrahedron(CScreensaverMicrocosm* base) : Gizmo(base)
  {
    for(int i=0; i<3; i++)
    {
      impSphere* sphere = new impSphere;
      sphere->setThickness(0.05f);
      mShapes.push_back(sphere);
    }

    for(int i=0; i<6; i++)
    {
      impCapsule* cap = new impCapsule;
      cap->setThickness(0.03f);
      cap->setLength(0.32f);
      mShapes.push_back(cap);

      rsMatrix m;
      m.makeScale(0.01f, 0.01f, 0.01f);
      cap->setMatrix(m.get());
    }
  }

  ~Tetrahedron()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    const float offset = 0.3f;
    rsMatrix m;
    for(int i=0; i<3; i++)
    {
      m.makeTranslate(offset * cosf(mCoeffPhase[i*2+1] * 5.0f), offset * cosf(mCoeffPhase[i*2+2] * 5.0f), offset * cosf(mCoeffPhase[i*2+3] * 5.0f));
      m.postMult(mMatrix);
      mShapes[i]->setPosition(&((m.get())[12]));
    }

    const float edge_face_angle = -0.61548f;
    const float radius_edge = 0.27f;
    impCapsule* cap = static_cast<impCapsule*>(mShapes[3]);
    m.makeTranslate(radius_edge, 0.0f, 0.0f);
    m.rotate(edge_face_angle, 0.0f, 1.0f, 0.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[4]);
    m.makeTranslate(radius_edge, 0.0f, 0.0f);
    m.rotate(edge_face_angle, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI * 0.66667f, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[5]);
    m.makeTranslate(radius_edge, 0.0f, 0.0f);
    m.rotate(edge_face_angle, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI * -0.66667f, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[6]);
    m.makeRotate(RS_PI * 0.5f, 1.0f, 0.0f, 0.0f);
    m.translate(radius_edge, 0.0f, 0.0f);
    m.rotate(RS_PI * 0.16667f, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[7]);
    m.makeRotate(RS_PI * 0.5f, 1.0f, 0.0f, 0.0f);
    m.translate(radius_edge, 0.0f, 0.0f);
    m.rotate(RS_PI * 0.16667f, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI * 0.33333f, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[8]);
    m.makeRotate(RS_PI * 0.5f, 1.0f, 0.0f, 0.0f);
    m.translate(radius_edge, 0.0f, 0.0f);
    m.rotate(RS_PI * 0.16667f, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI * -0.33333f, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());
  }
};
