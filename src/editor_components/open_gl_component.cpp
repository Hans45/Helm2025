/* Copyright 2013-2017 Matt Tytel
 *
 * helm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * helm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with helm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "open_gl_component.h"
#include "full_interface.h"

#ifndef NOMINMAX
// Prevent windows.h from defining the macros 'min' and 'max' which break
// std::min/std::max and other symbol names (they can also cause parsing
// errors such as unexpected tokens '::('). Define NOMINMAX before
// including <windows.h> to avoid these collisions.
#define NOMINMAX
#endif
#include <windows.h>
#include <juce_opengl/juce_opengl.h>

using namespace ::juce::gl;

void OpenGLComponent::setViewPort(OpenGLContext& open_gl_context) {
  float scale = open_gl_context.getRenderingScale();
  FullInterface* parent = findParentComponentOfClass<FullInterface>();
  juce::Rectangle<int> top_level_bounds = parent->getBounds();
  juce::Rectangle<int> global_bounds = parent->getLocalArea(this, getLocalBounds());

  glViewport(scale * global_bounds.getX(),
             scale * (top_level_bounds.getHeight() - global_bounds.getBottom()),
             scale * global_bounds.getWidth(), scale * global_bounds.getHeight());
}


