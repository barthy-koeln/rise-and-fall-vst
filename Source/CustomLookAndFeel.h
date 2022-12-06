#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <cstdlib>
#include <cmath>

class CustomLookAndFeel :
  public juce::LookAndFeel_V4 {
  public:
    juce::Colour COLOUR_BLACK;
    juce::Colour COLOUR_WHITE;
    juce::Colour COLOUR_RED;
    juce::String DIMENSION_MS;
    juce::String DIMENSION_PERCENT;
    juce::String DIMENSION_TIMES;
    juce::String DIMENSION_HERTZ;
    juce::String DIMENSION_FRACTION;

    CustomLookAndFeel () {
      COLOUR_BLACK = juce::Colour(0xff181818);
      COLOUR_WHITE = juce::Colour(0xffffffff);
      COLOUR_RED = juce::Colour(0xffCC3333);
      DIMENSION_MS = juce::CharPointer_UTF8(" ms");
      DIMENSION_PERCENT = juce::CharPointer_UTF8(" %");
      DIMENSION_TIMES = juce::CharPointer_UTF8(" x");
      DIMENSION_HERTZ = juce::CharPointer_UTF8(" Hz");
      DIMENSION_FRACTION = juce::CharPointer_UTF8("%d");
    };

    void drawRotarySlider (
      juce::Graphics &g,
      int x,
      [[maybe_unused]] int y,
      int width,
      int height,
      float sliderPos,
      const float rotaryStartAngle,
      const float rotaryEndAngle,
      juce::Slider &slider
    ) override {
      const float radius = (width / 2) - 16.0f;
      const float centreX = x + width * 0.5f;
      const float centreY = centreX;
      const float rx = centreX - radius;
      const float ry = centreY - radius;
      const float rw = radius * 2.0f;
      const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

      // fill
      g.setColour(COLOUR_BLACK);
      g.fillEllipse(rx, ry, rw, rw);

      // outline
      g.setColour(COLOUR_WHITE);
      g.drawEllipse(rx, ry, rw, rw, 5.0f);

      // pointer
      juce::Path p;
      const float pointerLength = 18.0f;
      const float pointerThickness = 5.0f;
      p.addRectangle(
        -pointerThickness * 0.5f,
        -radius,
        pointerThickness,
        pointerLength
      );
      p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
      g.setColour(COLOUR_RED);
      g.fillPath(p);

      g.setColour(COLOUR_WHITE);
      const int numLines = 10;
      const float lineThickness = 4.0f;
      const float lineAngleStep = (rotaryEndAngle - rotaryStartAngle) / numLines;
      const float lineLength = pointerLength * 0.5f;
      for (int i = 1; i < numLines; i++) {
        juce::Path line;
        const float lineAngle = rotaryStartAngle + (i * lineAngleStep);
        line.addRectangle(
          -lineThickness * 0.5f,
          -radius - lineLength - 6.0f,
          lineThickness,
          lineLength
        );
        line.applyTransform(
          juce::AffineTransform::rotation(lineAngle).translated(centreX, centreY));

        // lines
        g.fillPath(line);
      }

      g.setFont(10.0f);

      juce::String suffix = slider.getTextValueSuffix();
      juce::String labelEnd;
      juce::String labelStart;

      if (suffix.equalsIgnoreCase(DIMENSION_FRACTION)) {
        auto min = static_cast<short>(slider.getMinimum());
        auto max = static_cast<short>(slider.getMaximum());
        labelStart = "1/" + juce::String(pow(2, abs(min)));
        labelEnd = juce::String(pow(2, max));
      } else {
        labelStart = juce::String((int) slider.getMinimum()) + suffix;
        if (suffix.equalsIgnoreCase(DIMENSION_HERTZ)) {
          labelEnd = juce::String((int) (slider.getMaximum() / 1000)) + " kHz";
        } else if (suffix.equalsIgnoreCase(DIMENSION_MS)) {
          labelEnd = juce::String((int) (slider.getMaximum() / 1000)) + " s";
        } else {
          labelEnd = juce::String((int) slider.getMaximum()) + suffix;
        }
      }

      g.drawFittedText(
        labelStart,
        0,
        height - 26,
        width,
        12,
        juce::Justification::left,
        1
      );
      g.drawFittedText(
        labelEnd,
        0,
        height - 26,
        width,
        12,
        juce::Justification::right,
        1
      );

      g.setFont(12.0f);
      g.drawFittedText(
        slider.getName(),
        0,
        height - 12,
        width,
        12,
        juce::Justification::centred,
        1
      );
    }

    void drawComboBox (
      juce::Graphics &g,
      int width,
      int height,
      [[maybe_unused]] bool isMouseButtonDown,
      [[maybe_unused]] int buttonX,
      [[maybe_unused]] int buttonY,
      [[maybe_unused]] int buttonW,
      [[maybe_unused]] int buttonH,
      juce::ComboBox &box
    ) override {
      const juce::Rectangle<int> boxBounds(0, 16, width, 32);
      g.setColour(COLOUR_WHITE);
      g.fillRect(boxBounds);
      g.setFont(12.0f);
      g.drawFittedText(box.getName(), 0, 0, width, 12, juce::Justification::left, 1);

      juce::Rectangle<int> arrowZone(width - 30, 0, 20, height);
      juce::Path path;
      path.startNewSubPath(
        arrowZone.getX() + 3.0f,
        arrowZone.getCentreY() - 2.0f
      );
      path.lineTo(
        static_cast<float>(arrowZone.getCentreX()),
        arrowZone.getCentreY() + 3.0f
      );
      path.lineTo(arrowZone.getRight() - 3.0f, arrowZone.getCentreY() - 2.0f);

      g.setColour(COLOUR_BLACK);
      g.strokePath(path, juce::PathStrokeType(2.0f));

      box.setColour(juce::ComboBox::textColourId, COLOUR_BLACK);
    }

    void drawPopupMenuItem (
      juce::Graphics &g,
      const juce::Rectangle<int> &area,
      [[maybe_unused]] const bool isSeparator,
      [[maybe_unused]] const bool isActive,
      const bool isHighlighted,
      const bool isTicked,
      [[maybe_unused]] const bool hasSubMenu,
      const juce::String &text,
      [[maybe_unused]] const juce::String &shortcutKeyText,
      [[maybe_unused]] const juce::Drawable *icon,
      [[maybe_unused]] const juce::Colour *const textColourToUse
    ) override {
      auto r = area.reduced(1);

      if (isHighlighted) {
        g.setColour(COLOUR_WHITE);
      } else {
        g.setColour(COLOUR_BLACK);
      }
      g.fillRect(r);
      g.setColour(COLOUR_BLACK);

      r.reduce(juce::jmin(5, area.getWidth() / 20), 0);
      auto font = getPopupMenuFont();
      const auto maxFontHeight = r.getHeight() / 1.3f;

      if (font.getHeight() > maxFontHeight) {
        font.setHeight(maxFontHeight);
      }

      g.setFont(font);

      auto iconArea = r.removeFromLeft(juce::roundToInt(maxFontHeight / 2)).toFloat();
      if (isTicked) {
        juce::Path p;
        p.addEllipse(0, 0, 1, 1);
        if (isHighlighted) {
          g.setColour(COLOUR_BLACK);
        } else {
          g.setColour(COLOUR_WHITE);
        }
        g.fillPath(
          p,
          p.getTransformToScaleToFit(
            iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(),
            true
          )
        );
      }

      r.removeFromRight(3);

      if (isHighlighted) {
        g.setColour(COLOUR_BLACK);
      } else {
        g.setColour(COLOUR_WHITE);
      }
      g.drawFittedText(text, r, juce::Justification::centredLeft, 1);
    }

    void drawButtonBackground (
      juce::Graphics &g,
      juce::Button &button,
      [[maybe_unused]] const juce::Colour &backgroundColour,
      bool isMouseOverButton,
      bool isButtonDown
    ) override {
      const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

      auto baseColour =
        COLOUR_WHITE
          .withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
          .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

      if (isButtonDown || isMouseOverButton) {
        baseColour = baseColour.contrasting(isButtonDown ? 0.2f : 0.05f);
      }

      g.setColour(baseColour);
      g.fillRect(bounds);
    }

    void getIdealPopupMenuItemSize (
      const juce::String &text,
      const bool isSeparator,
      int standardMenuItemHeight,
      int &idealWidth,
      int &idealHeight
    ) override {
      if (isSeparator) {
        idealWidth = 50;
        idealHeight =
          standardMenuItemHeight > 0 ? standardMenuItemHeight / 10 : 10;
      } else {
        auto font = getPopupMenuFont();
        font.setHeight(16);

        idealHeight = 32;
        idealWidth = font.getStringWidth(text) + 32;
      }
    }

    void drawLinearSlider (
      juce::Graphics &g, int x, int y, int width, int height,
      float sliderPos,
      [[maybe_unused]] float minSliderPos,
      [[maybe_unused]] float maxSliderPos,
      [[maybe_unused]] const juce::Slider::SliderStyle style,
      juce::Slider &slider
    ) override {
      auto trackWidth =
        juce::jmin(6.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);

      juce::Point<float> startPoint(
        slider.isHorizontal() ? x : x + width * 0.5f,
        slider.isHorizontal() ? y + height * 0.5f : height + y
      );

      juce::Point<float> endPoint(
        slider.isHorizontal() ? width + x : startPoint.x,
        slider.isHorizontal() ? startPoint.y : y
      );

      juce::Path backgroundTrack;
      backgroundTrack.startNewSubPath(startPoint);
      backgroundTrack.lineTo(endPoint);
      g.setColour(COLOUR_WHITE.withMultipliedAlpha(0.5));
      g.strokePath(
        backgroundTrack,
        {trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded}
      );

      juce::Path valueTrack;
      juce::Point<float> minPoint, maxPoint;

      auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
      auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;

      minPoint = startPoint;
      maxPoint = {kx, ky};

      valueTrack.startNewSubPath(minPoint);
      valueTrack.lineTo(maxPoint);
      g.setColour(COLOUR_WHITE);
      g.strokePath(
        valueTrack,
        {
          trackWidth,
          juce::PathStrokeType::curved,
          juce::PathStrokeType::rounded
        }
      );

      auto thumbWidth = getSliderThumbRadius(slider);

      g.setColour(COLOUR_RED);
      g.fillEllipse(
        juce::Rectangle<float>(
          static_cast<float>(thumbWidth),
          static_cast<float>(thumbWidth)).withCentre(maxPoint)
      );

      g.setFont(12.0f);
      g.setColour(COLOUR_WHITE);
      g.drawFittedText(
        slider.getName(),
        12,
        0,
        width,
        12,
        juce::Justification::left,
        1
      );
    }
};
