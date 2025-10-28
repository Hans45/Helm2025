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

#include "wave_selector.h"

#define TYPE_PADDING_X 1.0f
#define TYPE_PADDING_Y 4.0f

namespace {
  template<size_t steps>
  void resizeSteps(Path& path, float x, float y, float width, float height) {
    path.clear();
    path.startNewSubPath(x, y + height / 2.0f);

    float inc_x = width / steps;
    float inc_y = height / (steps - 1);
    for (int i = 0; i < steps; ++i) {
      path.lineTo(x + i * inc_x, y + height - i * inc_y);
      path.lineTo(x + (i + 1) * inc_x, y + height - i * inc_y);
    }

    path.lineTo(x + width, y + height / 2.0f);
  }

  template<size_t steps>
  void resizePyramid(Path& path, float x, float y, float width, float height) {
    static int parts = 2 * (steps - 1);

    path.clear();
    path.startNewSubPath(x, y + height / 2.0f);

    float inc_x = width / parts;
    float inc_y = height / (steps - 1);
    float cur_x = x + inc_x;

    for (int i = 0; i < (steps - 1) / 2; ++i) {
      path.lineTo(cur_x, y + height / 2.0f - i * inc_y);
      path.lineTo(cur_x, y + height / 2.0f - (i + 1) * inc_y);
      cur_x += inc_x;
    }

    for (int i = 0; i < steps - 1; ++i) {
      path.lineTo(cur_x, y + i * inc_y);
      path.lineTo(cur_x, y + (i + 1) * inc_y);
      cur_x += inc_x;
    }

    for (int i = 0; i < (steps - 1) / 2; ++i) {
      path.lineTo(cur_x, y + height - i * inc_y);
      path.lineTo(cur_x, y + height - (i + 1) * inc_y);
      cur_x += inc_x;
    }

    path.lineTo(x + width, y + height / 2.0f);
  }
} // namespace

WaveSelector::WaveSelector(String name) : SynthSlider(name) { }

void WaveSelector::paint(Graphics& g) {
  static const PathStrokeType stroke(1.000f, PathStrokeType::curved, PathStrokeType::rounded);

  int selected = getValue();
  int num_types = getMaximum() - getMinimum() + 1;
  float cell_width = float(getWidth()) / num_types;
  g.setColour(Colour(0xff424242));
  g.fillRect(selected * cell_width, 0.0f, cell_width, float(getHeight()));

  // Original waveforms (0-11)
  g.setColour(selected == 0 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(sine_, stroke);

  g.setColour(selected == 1 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(triangle_, stroke);

  g.setColour(selected == 2 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(square_, stroke);

  g.setColour(selected == 3 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(down_saw_, stroke);

  g.setColour(selected == 4 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(up_saw_, stroke);

  g.setColour(selected == 5 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(three_step_, stroke);

  g.setColour(selected == 6 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(four_step_, stroke);

  g.setColour(selected == 7 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(eight_step_, stroke);

  g.setColour(selected == 8 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(three_pyramid_, stroke);

  g.setColour(selected == 9 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(five_pyramid_, stroke);

  g.setColour(selected == 10 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(nine_pyramid_, stroke);

  g.setColour(selected == 11 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(noise_, stroke);

  // New waveforms (12-20)
  g.setColour(selected == 12 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(pulse25_, stroke);

  g.setColour(selected == 13 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(pulse10_, stroke);

  g.setColour(selected == 14 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(saw_square_, stroke);

  g.setColour(selected == 15 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(triangle_square_, stroke);

  g.setColour(selected == 16 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(skewed_sine_, stroke);

  g.setColour(selected == 17 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(folded_sine_, stroke);

  g.setColour(selected == 18 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(super_saw_, stroke);

  g.setColour(selected == 19 ? Colour(0xffffffff) : Colour(0xffaaaaaa));
  g.strokePath(chirp_, stroke);
}

void WaveSelector::resized() {
  SynthSlider::resized();

  int num_types = getMaximum() - getMinimum() + 1;
  float cell_width = float(getWidth()) / num_types;
  float type_width = cell_width - 2 * TYPE_PADDING_X;
  float type_height = getHeight() - 2 * TYPE_PADDING_Y;

  // Original waveforms (0-11)
  resizeSin(0.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeTriangle(1.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeSquare(2.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeDownSaw(3.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeUpSaw(4.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeSteps<3>(three_step_, 5.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y,
  type_width, type_height);
  resizeSteps<4>(four_step_, 6.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y,
  type_width, type_height);
  resizeSteps<8>(eight_step_, 7.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y,
  type_width, type_height);
  resizePyramid<3>(three_pyramid_, 8.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y,
  type_width, type_height);
  resizePyramid<5>(five_pyramid_, 9.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y,
  type_width, type_height);
  resizePyramid<9>(nine_pyramid_, 10.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y,
  type_width, type_height);
  resizeNoise(11.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);

  // New waveforms (12-19)
  resizePulse25(12.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizePulse10(13.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeSawSquare(14.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeTriangleSquare(15.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeSkewedSine(16.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeFoldedSine(17.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeSuperSaw(18.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
  resizeChirp(19.0f * cell_width + TYPE_PADDING_X, TYPE_PADDING_Y, type_width, type_height);
}

void WaveSelector::mouseEvent(const juce::MouseEvent &e) {
  float x = e.getPosition().getX();
  float width = getWidth();
  int current = getValue();
  int max = getMaximum();
  // Moitié gauche = précédent, moitié droite = suivant
  if (x < width / 2.0f) {
    setValue((current - 1 + (max + 1)) % (max + 1));
  } else {
    setValue((current + 1) % (max + 1));
  }
}

void WaveSelector::mouseDown(const juce::MouseEvent &e) {
  mouseEvent(e);
}

void WaveSelector::mouseDrag(const juce::MouseEvent &e) {
  mouseEvent(e);
}

void WaveSelector::resizeSin(float x, float y, float width, float height) {
  sine_.clear();
  sine_.startNewSubPath(x, y + height / 2.0f);
  sine_.lineTo(x + width / 8.0f, y + height / 6.0f);
  sine_.lineTo(x + 2.0f * width / 8.0f, y);
  sine_.lineTo(x + 3.0f * width / 8.0f, y + height / 6.0f);
  sine_.lineTo(x + 5.0f * width / 8.0f, y + 5.0f * height / 6.0f);
  sine_.lineTo(x + 6.0f * width / 8.0f, y + height);
  sine_.lineTo(x + 7.0f * width / 8.0f, y + 5.0f * height / 6.0f);
  sine_.lineTo(x + 8.0f * width / 8.0f, y + height / 2.0f);
}

void WaveSelector::resizeTriangle(float x, float y, float width, float height) {
  triangle_.clear();
  triangle_.startNewSubPath(x, y + height / 2.0f);
  triangle_.lineTo(x + 1.0f * width / 4.0f, y);
  triangle_.lineTo(x + 3.0f * width / 4.0f, y + height);
  triangle_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizeSquare(float x, float y, float width, float height) {
  square_.clear();
  square_.startNewSubPath(x, y + height / 2.0f);
  square_.lineTo(x, y);
  square_.lineTo(x + width / 2.0f, y);
  square_.lineTo(x + width / 2.0f, y + height);
  square_.lineTo(x + width, y + height);
  square_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizeDownSaw(float x, float y, float width, float height) {
  down_saw_.clear();
  down_saw_.startNewSubPath(x, y + height / 2.0f);
  down_saw_.lineTo(x, y);
  down_saw_.lineTo(x + width, y + height);
  down_saw_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizeUpSaw(float x, float y, float width, float height) {
  up_saw_.clear();
  up_saw_.startNewSubPath(x, y + height / 2.0f);
  up_saw_.lineTo(x, y + height);
  up_saw_.lineTo(x + width, y);
  up_saw_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizeNoise(float x, float y, float width, float height) {
  static const int noise_icon_resolution = 14;
  srand(0);
  noise_.clear();
  noise_.startNewSubPath(x, y + height / 2.0f);
  float inc_x = width / noise_icon_resolution;

  for (int i = 1; i < noise_icon_resolution; ++i)
    noise_.lineTo(x + i * inc_x, y + height * (rand() % 100) / 100.0f);
  noise_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizePulse25(float x, float y, float width, float height) {
  pulse25_.clear();
  pulse25_.startNewSubPath(x, y + height / 2.0f);
  pulse25_.lineTo(x, y);
  pulse25_.lineTo(x + width / 4.0f, y);  // 25% duty cycle
  pulse25_.lineTo(x + width / 4.0f, y + height);
  pulse25_.lineTo(x + width, y + height);
  pulse25_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizePulse10(float x, float y, float width, float height) {
  pulse10_.clear();
  pulse10_.startNewSubPath(x, y + height / 2.0f);
  pulse10_.lineTo(x, y);
  pulse10_.lineTo(x + width / 10.0f, y);  // 10% duty cycle
  pulse10_.lineTo(x + width / 10.0f, y + height);
  pulse10_.lineTo(x + width, y + height);
  pulse10_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizeSawSquare(float x, float y, float width, float height) {
  saw_square_.clear();
  saw_square_.startNewSubPath(x, y + height / 2.0f);
  // Hybrid: start with saw
  saw_square_.lineTo(x, y + height * 0.8f);  // 80% down (saw component)
  saw_square_.lineTo(x + width / 3.0f, y + height * 0.2f);  // up slope
  // Square component
  saw_square_.lineTo(x + width / 3.0f, y);
  saw_square_.lineTo(x + 2.0f * width / 3.0f, y);
  saw_square_.lineTo(x + 2.0f * width / 3.0f, y + height);
  saw_square_.lineTo(x + width, y + height);
  saw_square_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizeTriangleSquare(float x, float y, float width, float height) {
  triangle_square_.clear();
  triangle_square_.startNewSubPath(x, y + height / 2.0f);
  // Triangle component (70%)
  triangle_square_.lineTo(x + width / 4.0f, y + height * 0.1f);
  triangle_square_.lineTo(x + width / 2.0f, y + height * 0.9f);
  // Square component (30%)
  triangle_square_.lineTo(x + width / 2.0f, y);
  triangle_square_.lineTo(x + 3.0f * width / 4.0f, y);
  triangle_square_.lineTo(x + 3.0f * width / 4.0f, y + height);
  triangle_square_.lineTo(x + width, y + height);
  triangle_square_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizeSkewedSine(float x, float y, float width, float height) {
  skewed_sine_.clear();
  skewed_sine_.startNewSubPath(x, y + height / 2.0f);

  // Asymmetric sine with faster rise, slower fall
  int points = 16;
  for (int i = 1; i < points; ++i) {
    float t = float(i) / points;
    float skewed_t = t < 0.5f ? t * t * 2.0f : 1.0f - (1.0f - t) * (1.0f - t) * 2.0f;
    float sine_val = sin(2.0f * 3.14159f * skewed_t);
    skewed_sine_.lineTo(x + i * width / points, y + height / 2.0f - sine_val * height / 2.0f);
  }
  skewed_sine_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizeFoldedSine(float x, float y, float width, float height) {
  folded_sine_.clear();
  folded_sine_.startNewSubPath(x, y + height / 2.0f);

  // Sine with folding at �0.5
  int points = 16;
  for (int i = 1; i < points; ++i) {
    float t = float(i) / points;
    float sine_val = sin(2.0f * 3.14159f * t);
    // Apply folding
    if (sine_val > 0.5f) sine_val = 1.0f - sine_val;
    else if (sine_val < -0.5f) sine_val = -1.0f - sine_val;
    folded_sine_.lineTo(x + i * width / points, y + height / 2.0f - sine_val * height / 2.0f);
  }
  folded_sine_.lineTo(x + width, y + height / 2.0f);
}

void WaveSelector::resizeSuperSaw(float x, float y, float width, float height) {
  super_saw_.clear();
  super_saw_.startNewSubPath(x, y + height / 2.0f);

  // Multiple overlapping saws with slight phase differences
  for (int voice = 0; voice < 3; ++voice) {  // Show 3 of the 7 voices for clarity
    float phase_offset = voice * 0.1f;
    float y_offset = voice * height * 0.05f;  // Slight vertical separation

    Path single_saw;
    single_saw.startNewSubPath(x + phase_offset * width, y + height / 2.0f + y_offset);
    single_saw.lineTo(x + phase_offset * width, y + height * 0.9f + y_offset);
    single_saw.lineTo(x + width + phase_offset * width, y + height * 0.1f + y_offset);

    super_saw_.addPath(single_saw);
  }
}

void WaveSelector::resizeChirp(float x, float y, float width, float height) {
  chirp_.clear();
  chirp_.startNewSubPath(x, y + height / 2.0f);

  // Frequency sweep visualization
  int points = 32;
  for (int i = 1; i < points; ++i) {
    float t = float(i) / points;
    float frequency_mult = 1.0f + 4.0f * t;  // 1x to 5x frequency
    float sine_val = sin(2.0f * 3.14159f * t * frequency_mult);
    chirp_.lineTo(x + i * width / points, y + height / 2.0f - sine_val * height / 2.0f);
  }
  chirp_.lineTo(x + width, y + height / 2.0f);
}


