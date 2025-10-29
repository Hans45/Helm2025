/* Copyright 2013-2017 Matt Tytel
 * helm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
// Fin correcte de setWaveSlider, suppression de l'accolade en trop
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

#include "open_gl_wave_viewer.h"

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

#include "colors.h"
#include "synth_gui_interface.h"
#include "utils.h"
#include "../synthesis/synced_random.h"

#define GRID_CELL_WIDTH 8
#include "../synthesis/helm2025_lfo.h"
#include "../synthesis/synced_random.h"
#define PADDING 5.0f
#define MARKER_WIDTH 12.0f
#define NOISE_RESOLUTION 6
#define IMAGE_HEIGHT 256

OpenGLWaveViewer::OpenGLWaveViewer(int resolution) {
  wave_slider_ = nullptr;
  amplitude_slider_ = nullptr;
  resolution_ = resolution;
  wave_phase_ = nullptr;
  wave_amp_ = nullptr;
  last_phase_ = 0.0f;

  // synced_randoms_ sera généré dynamiquement selon le LFO

  position_vertices_ = new float[16] {
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, -1.0f, 0.0f, 0.0f,
    0.1f, -1.0f, 1.0f, 0.0f,
    0.1f, 1.0f, 1.0f, 1.0f
  };


  position_triangles_ = new int[6] {
    0, 1, 2,
    2, 3, 0
  };
}

OpenGLWaveViewer::~OpenGLWaveViewer() {
  delete[] position_vertices_;
  delete[] position_triangles_;
}

void OpenGLWaveViewer::paintBackground() {
  static const DropShadow shadow(Colour(0xbb000000), 5, Point<int>(0, 0));

  if (getWidth() <= 0 || getHeight() <= 0)
    return;

  auto *display = Desktop::getInstance().getDisplays().getPrimaryDisplay();
  jassert(display != nullptr);

  float scale = display->scale;
  background_image_ = Image(Image::ARGB, scale * getWidth(), scale * getHeight(), true);
  Graphics g(background_image_);
  g.addTransform(AffineTransform::scale(scale, scale));

  // Fond neutre
  g.fillAll(Colour(0xff424242));
  g.setColour(Colour(0xff4a4a4a));
  for (int x = 0; x < getWidth(); x += GRID_CELL_WIDTH)
    g.drawLine(x, 0, x, getHeight());
  for (int y = 0; y < getHeight(); y += GRID_CELL_WIDTH)
    g.drawLine(0, y, getWidth(), y);

  g.setColour(Colors::graph_fill);
  g.fillPath(wave_path_);

  g.setColour(Colors::modulation);
  float line_width = 1.5f * getHeight() / 75.0f;
  PathStrokeType stroke(line_width, PathStrokeType::beveled, PathStrokeType::rounded);
  g.strokePath(wave_path_, stroke);

  background_.updateBackgroundImage(background_image_);
}

void OpenGLWaveViewer::paintPositionImage() {
  int min_image_width = roundToInt(2 * MARKER_WIDTH);
  int image_width = mopo::utils::nextPowerOfTwo(min_image_width);
  int marker_width = MARKER_WIDTH;
  int image_height = roundToInt(2 * IMAGE_HEIGHT);
  position_image_ = Image(Image::ARGB, image_width, image_height, true);
  Graphics g(position_image_);

  g.setColour(Colour(0x77ffffff));
  g.fillRect(image_width / 2.0f - 0.5f, 0.0f, 1.0f, 1.0f * image_height);

  g.setColour(Colors::modulation);
  g.fillEllipse(image_width / 2 - marker_width / 2, image_height / 2 - marker_width / 2,
                marker_width, marker_width);
  g.setColour(Colour(0xff000000));
  g.fillEllipse(image_width / 2 - marker_width / 4, image_height / 2 - marker_width / 4,
                marker_width / 2, marker_width / 2);
}

void OpenGLWaveViewer::mouseDown(const MouseEvent& e) {
  if (wave_slider_) {
    int current_value = wave_slider_->getValue();
    int max_value = static_cast<int>(wave_slider_->getMaximum());
    int new_value = current_value;
    // Clic gauche = précédent, clic droit = suivant
    if (e.x < getWidth() / 2) {
      // Précédent
      new_value = (current_value - 1 + (max_value + 1)) % (max_value + 1);
    } else {
      // Suivant
      new_value = (current_value + 1) % (max_value + 1);
    }
    wave_slider_->setValue(new_value);
    resetWavePath();
  }
}

void OpenGLWaveViewer::resized() {
  resetWavePath();

  SynthGuiInterface* parent = findParentComponentOfClass<SynthGuiInterface>();
  if (wave_amp_ == nullptr && parent) {
    wave_amp_ = parent->getSynth()->getModSource(getName().toStdString() + "_amp");

    if (wave_amp_ == nullptr)
      wave_amp_ = parent->getSynth()->getModSource(getName().toStdString());
  }

  if (wave_phase_ == nullptr && parent)
    wave_phase_ = parent->getSynth()->getModSource(getName().toStdString() + "_phase");
}

void OpenGLWaveViewer::setWaveSlider(SynthSlider* slider) {
  wave_slider_ = slider;
  if (wave_slider_)
    wave_slider_->addSliderListener(this);
  resetWavePath();
}

void OpenGLWaveViewer::setAmplitudeSlider(SynthSlider* slider) {
  amplitude_slider_ = slider;
  amplitude_slider_->addSliderListener(this);
  resetWavePath();
}

void OpenGLWaveViewer::drawRandom() {
  // Affichage S&H/S&G : n steps par cycle, chaque step = segment horizontal
  float amplitude = amplitude_slider_ ? amplitude_slider_->getValue() : 1.0f;
  float draw_width = getWidth();
  float padding = getRatio() * PADDING;
  float draw_height = getHeight() - 2.0f * padding;
  int n = static_cast<int>(synced_randoms_.size());
  if (n <= 0) return; // <-- AJOUT DU GARDE-FOU
  float step_width = draw_width / n;
  wave_path_.startNewSubPath(0, getHeight() / 2.0f);
  for (int i = 0; i < n; ++i) {
    float val = amplitude * synced_randoms_[i];
    float x1 = i * step_width;
    float x2 = (i + 1) * step_width;
    float y = padding + draw_height * ((1.0f - val) / 2.0f);
    wave_path_.lineTo(x1, y);
    wave_path_.lineTo(x2, y);
  }
  wave_path_.lineTo(getWidth(), getHeight() / 2.0f);
}

void OpenGLWaveViewer::drawSmoothRandom() {
  float amplitude = amplitude_slider_ ? amplitude_slider_->getValue() : 1.0f;
  float draw_width = getWidth();
  float padding = getRatio() * PADDING;
  float draw_height = getHeight() - 2.0f * padding;
  int n = cycle_resolution_;
  if (n <= 0 || synced_randoms_.empty()) return;
  wave_path_.startNewSubPath(-50, getHeight() / 2.0f);
  for (int i = 0; i < resolution_; ++i) {
    float t = (1.0f * i) / resolution_;
    float phase = t * (n - 1);
    int index = (int)phase;
    float frac = phase - index;
    float val = amplitude * mopo::utils::interpolate(
      synced_randoms_[index % synced_randoms_.size()],
      synced_randoms_[(index + 1) % synced_randoms_.size()],
      static_cast<float>(frac));
    wave_path_.lineTo(t * draw_width, padding + draw_height * ((1.0f - val) / 2.0f));
  }
  float end_val = amplitude * synced_randoms_[(n - 1) % synced_randoms_.size()];
  wave_path_.lineTo(getWidth(), padding + draw_height * ((1.0f - end_val) / 2.0f));
  wave_path_.lineTo(getWidth() + 50, getHeight() / 2.0f);
}



void OpenGLWaveViewer::resetWavePath() {
  wave_path_.clear();
  // Toujours clear le chemin pour éviter l'affichage résiduel
  wave_path_.clear();
  if (wave_slider_ == nullptr)
    return;

  float amplitude = amplitude_slider_ ? amplitude_slider_->getValue() : 1.0f;
  float draw_width = getWidth();
  float padding = getRatio() * PADDING;
  float draw_height = getHeight() - 2.0f * padding;

  mopo::Wave::Type type = static_cast<mopo::Wave::Type>(static_cast<int>(wave_slider_->getValue()));

  // --- NOUVEAU : visualisation statique pour Poly LFO S&H/S&G ---
  bool isPolyLfo = getName().containsIgnoreCase("poly_lfo");
  static mopo::Wave::Type lastType = mopo::Wave::kSin;
  static bool lastIsPoly = false;
  if (isPolyLfo && (type == mopo::Wave::kSampleAndHold || type == mopo::Wave::kSampleAndGlide)) {
    // Toujours régénérer la séquence random statique à chaque appel
    static uint32_t static_seed = 123456;
    static int static_res = 16;
    cycle_seed_ = static_seed;
    cycle_resolution_ = static_res;
    synced_randoms_ = mopo::generateSyncedRandoms(cycle_seed_, cycle_resolution_);
    lastType = type;
    lastIsPoly = true;
    if (cycle_resolution_ > 0 && !synced_randoms_.empty()) {
      if (type == mopo::Wave::kSampleAndGlide)
        drawSmoothRandom();
      else
        drawRandom();
    }
    paintBackground();
    repaint();
    return;
  } else {
    lastIsPoly = false;
  }

  // --- Cas dynamique normal (synchro LFO) ---
  if (type == mopo::Wave::kSampleAndHold || type == mopo::Wave::kWhiteNoise || type == mopo::Wave::kSampleAndGlide) {
    // Synchronisation stricte avec le LFO
    SynthGuiInterface* parent = findParentComponentOfClass<SynthGuiInterface>();
    mopo::HelmLfo* lfo = nullptr;
    if (parent) {
      auto* synth = parent->getSynth();
      if (getName().containsIgnoreCase("poly_lfo")) {
        if (synth && synth->getEngine()) {
          lfo = synth->getEngine()->getPolyLfo();
        }
      } else if (getName().containsIgnoreCase("lfo_1")) {
        if (synth && synth->getEngine()) {
          lfo = synth->getEngine()->getLfo1();
        }
      } else if (getName().containsIgnoreCase("lfo_2")) {
        if (synth && synth->getEngine()) {
          lfo = synth->getEngine()->getLfo2();
        }
      }
    }
    if (lfo) {
      cycle_seed_ = lfo->getCycleSeed();
      cycle_resolution_ = lfo->getCycleResolution();
    } else {
      cycle_seed_ = 0;
      cycle_resolution_ = std::max(8, std::min(512, resolution_));
    }
    if (cycle_resolution_ <= 0) return;
    synced_randoms_ = mopo::generateSyncedRandoms(cycle_seed_, cycle_resolution_);
    if (synced_randoms_.empty() || !wave_phase_ || !amplitude_slider_) return;
  if (cycle_resolution_ > 0 && !synced_randoms_.empty()) {
        if (type == mopo::Wave::kSampleAndGlide)
            drawSmoothRandom();
        else
            drawRandom();
    }
    paintBackground();
    return;
  } else {
    // All deterministic waveforms (original + new ones)
    wave_path_.startNewSubPath(0, getHeight() / 2.0f);
    for (int i = 1; i < resolution_ - 1; ++i) {
      float t = (1.0f * i) / resolution_;
      float val = amplitude * mopo::Wave::wave(type, t);
      wave_path_.lineTo(t * draw_width, padding + draw_height * ((1.0f - val) / 2.0f));
    }
  }

  wave_path_.lineTo(getWidth(), getHeight() / 2.0f);
  paintBackground();
}

void OpenGLWaveViewer::guiChanged(SynthSlider* slider) {
  resetWavePath();
}

float OpenGLWaveViewer::phaseToX(float phase) {
  return phase * getWidth();
}

void OpenGLWaveViewer::init(OpenGLContext& open_gl_context) {
  paintPositionImage();

  open_gl_context.extensions.glGenBuffers(1, &vertex_buffer_);
  open_gl_context.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);

  GLsizeiptr vert_size = static_cast<GLsizeiptr>(static_cast<size_t>(16 * sizeof(float)));
  open_gl_context.extensions.glBufferData(GL_ARRAY_BUFFER, vert_size,
                                          position_vertices_, GL_STATIC_DRAW);

  open_gl_context.extensions.glGenBuffers(1, &triangle_buffer_);
  open_gl_context.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer_);

  GLsizeiptr tri_size = static_cast<GLsizeiptr>(static_cast<size_t>(6 * sizeof(float)));
  open_gl_context.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, tri_size,
                                          position_triangles_, GL_STATIC_DRAW);

  background_.init(open_gl_context);
}

void OpenGLWaveViewer::drawPosition(OpenGLContext& open_gl_context) {
  if (position_texture_.getWidth() != position_image_.getWidth())
    position_texture_.loadImage(position_image_);

  // Protection : on ne dessine rien si le buffer random est vide ou si wave_phase_ ou amplitude_slider_ sont nuls

  // --- Synchronisation stricte du random S&H/S&G : on vérifie le seed à chaque frame ---
  if (wave_slider_) {
    SynthGuiInterface* parent = findParentComponentOfClass<SynthGuiInterface>();
    mopo::HelmLfo* lfo = nullptr;
    if (parent) {
      auto* synth = parent->getSynth();
      if (getName().containsIgnoreCase("poly_lfo")) {
        if (synth && synth->getEngine()) {
          lfo = synth->getEngine()->getPolyLfo();
        }
      } else if (getName().containsIgnoreCase("lfo_1")) {
        if (synth && synth->getEngine()) {
          lfo = synth->getEngine()->getLfo1();
        }
      } else if (getName().containsIgnoreCase("lfo_2")) {
        if (synth && synth->getEngine()) {
          lfo = synth->getEngine()->getLfo2();
        }
      }
    }
    if (lfo) {
      uint32_t new_seed = lfo->getCycleSeed();
      int new_res = lfo->getCycleResolution();
      if (new_seed != cycle_seed_ || new_res != cycle_resolution_) {
        cycle_seed_ = new_seed;
        cycle_resolution_ = new_res;
        synced_randoms_ = mopo::generateSyncedRandoms(cycle_seed_, cycle_resolution_);
      }
    }
  }
  if (synced_randoms_.empty() || !wave_phase_ || !amplitude_slider_)
    return;

  if (wave_phase_ == nullptr || wave_amp_ == nullptr || wave_phase_->buffer[0] <= 0.0)
    return;

  // Detect cycle reset for Sample & Hold / Sample & Glide waveforms
  float current_phase = wave_phase_->buffer[0];
  if (wave_slider_ && (current_phase < last_phase_)) {
    // Phase wrapped around - new cycle started
    mopo::Wave::Type type = static_cast<mopo::Wave::Type>(static_cast<int>(wave_slider_->getValue()));
    if (type == mopo::Wave::kSampleAndHold || type == mopo::Wave::kSampleAndGlide || type == mopo::Wave::kWhiteNoise) {
  resetWavePath();
    }
  }
  last_phase_ = current_phase;

  float x = 2.0f * wave_phase_->buffer[0] - 1.0f;
  float padding = getRatio() * PADDING;
  
  // For Sample & Hold/Glide, calculate Y from the visual random values
  mopo::Wave::Type type = wave_slider_ ? static_cast<mopo::Wave::Type>(static_cast<int>(wave_slider_->getValue())) : mopo::Wave::kSin;
  float visual_amp;
  
  if (type == mopo::Wave::kSampleAndHold || type == mopo::Wave::kWhiteNoise) {
    // Pour S&H/WhiteNoise, la valeur doit rester constante sur chaque step audio
    // On force l'index à être borné strictement à [0, n-1] (jamais n)
    int n = static_cast<int>(synced_randoms_.size());
    float phase = wave_phase_->buffer[0];
    if (phase < 0.0f) phase = 0.0f;
    if (phase >= 1.0f) phase = std::nextafter(1.0f, 0.0f); // Jamais 1.0f pile
    int step = static_cast<int>(phase * n);
    if (step >= n) step = n - 1;
    float amplitude = amplitude_slider_ ? amplitude_slider_->getValue() : 1.0f;
    visual_amp = amplitude * synced_randoms_[step];
  }
  else if (type == mopo::Wave::kSampleAndGlide) {
    // Pour S&G, interpolation identique à l'audio (phase bornée)
    int n = static_cast<int>(synced_randoms_.size());
    float phase = wave_phase_->buffer[0];
    if (phase < 0.0f) phase = 0.0f;
    if (phase > 1.0f) phase = 1.0f;
    float interp = phase * (n - 1);
    int index = static_cast<int>(interp);
    float frac = interp - index;
    if (index >= n - 1) {
      index = n - 2;
      frac = 1.0f;
    }
    float t = (1.0f - cosf(mopo::PI * frac)) / 2.0f;
    float amplitude = amplitude_slider_ ? amplitude_slider_->getValue() : 1.0f;
    visual_amp = amplitude * mopo::utils::interpolate(synced_randoms_[index], synced_randoms_[index + 1], t);
  }
  else {
    // Pour les autres formes, on affiche la valeur réelle du LFO
    visual_amp = wave_amp_->buffer[0];
  }
  
  // Convert amplitude from -1..1 to OpenGL coordinates -1..1, accounting for padding
  float normalized_height = (getHeight() - 2 * padding) / (float)getHeight();
  float y = visual_amp * normalized_height;  // Removed the minus sign - it was inverting the position

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  int draw_width = getWidth();
  int draw_height = getHeight();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  float ratio = getHeight() / 75.0f;

  float position_height = ratio * (0.5f * position_texture_.getHeight()) / draw_height;
  float position_width = ratio * (0.5f * position_texture_.getWidth()) / draw_width;
  position_vertices_[0] = x - position_width;
  position_vertices_[1] = y + position_height;
  position_vertices_[4] = x - position_width;
  position_vertices_[5] = y - position_height;
  position_vertices_[8] = x + position_width;
  position_vertices_[9] = y - position_height;
  position_vertices_[12] = x + position_width;
  position_vertices_[13] = y + position_height;

  open_gl_context.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  GLsizeiptr vert_size = static_cast<GLsizeiptr>(static_cast<size_t>(16 * sizeof(float)));
  open_gl_context.extensions.glBufferData(GL_ARRAY_BUFFER, vert_size,
                                          position_vertices_, GL_STATIC_DRAW);

  open_gl_context.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_buffer_);
  position_texture_.bind();

  open_gl_context.extensions.glActiveTexture(GL_TEXTURE0);

  if (background_.texture_uniform() != nullptr)
    background_.texture_uniform()->set(0);

  background_.shader()->use();

  background_.enableAttributes(open_gl_context);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  background_.disableAttributes(open_gl_context);

  position_texture_.unbind();

  open_gl_context.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
  open_gl_context.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGLWaveViewer::render(OpenGLContext& open_gl_context, bool animate) {
  MOPO_ASSERT(glGetError() == GL_NO_ERROR);

  setViewPort(open_gl_context);

  background_.render(open_gl_context);

  if (animate)
    drawPosition(open_gl_context);

  MOPO_ASSERT(glGetError() == GL_NO_ERROR);
}

void OpenGLWaveViewer::destroy(OpenGLContext& open_gl_context) {
  position_texture_.release();

  texture_ = nullptr;
  open_gl_context.extensions.glDeleteBuffers(1, &vertex_buffer_);
  open_gl_context.extensions.glDeleteBuffers(1, &triangle_buffer_);
  background_.destroy(open_gl_context);
}

float OpenGLWaveViewer::getRatio() {
  return getHeight() / 80.0f;
}


