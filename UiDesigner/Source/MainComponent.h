/*
  ==============================================================================

    MainComponent.h
    Created: 11 Nov 2024 10:52:48pm
    Author:  Martin S

  ==============================================================================
*/

// MainComponent.h
#pragma once
#include <JuceHeader.h>

class StrokePatternButton : public juce::Button
{
public:
    StrokePatternButton(const juce::String& name);
    
    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StrokePatternButton)
};

class MainComponent : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::ChangeListener
{
public:
    enum class Tool
    {
        Rectangle,
        Ellipse,
        Line,
        Select
    };

    enum class StrokePattern
    {
        Solid,
        Dashed,
        Dotted,
        DashDot
    };

    struct Style
    {
        juce::Colour fillColour;
        juce::Colour strokeColour;
        float strokeWidth;
        StrokePattern strokePattern;
        bool hasFill;
        float cornerRadius = 0.0f;  // Add this line
    };

    struct Shape
    {
        Tool type;
        juce::Rectangle<float> bounds;
        juce::Point<float> startPoint;
        juce::Point<float> endPoint;
        Style style;
    };

    MainComponent();
    

    void paint(juce::Graphics& g) override;
     
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;

    void mouseDrag(const juce::MouseEvent& e) override;

    void mouseUp(const juce::MouseEvent& e) override;

    void sliderValueChanged(juce::Slider* slider) override;
    
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    void showColorPicker(bool isFillColor);

    void applyStyle(juce::Graphics& g, const Style& style);

    template<typename PathFunction>
    void drawStrokedPath(juce::Graphics& g, const Style& style, PathFunction&& pathFunc);

    // UI Components
    juce::TextButton rectangleButton;
    juce::TextButton ellipseButton;
    juce::TextButton lineButton;
    
    juce::Slider strokeWidthSlider;
    juce::Label strokeWidthLabel;
    
    juce::TextButton fillColorButton{ "Fill Color" };
    juce::TextButton strokeColorButton{ "Stroke Color" };
    juce::Label fillColorLabel;
    juce::Label strokeColorLabel;
    juce::Slider cornerRadiusSlider;
    juce::Label cornerRadiusLabel;
    
    StrokePatternButton solidStrokeButton{ "Solid" };
    StrokePatternButton dashedStrokeButton{ "Dashed" };
    StrokePatternButton dottedStrokeButton{ "Dotted" };
    StrokePatternButton dashDotStrokeButton{ "Dash-Dot" };
    
    juce::ToggleButton fillToggle;

    // State
    Tool currentTool = Tool::Rectangle;
    Style currentStyle;
    bool isDrawing = false;
    bool currentlyEditingFillColour = false;
    juce::Point<float> dragStart;
    juce::Point<float> dragEnd;
    juce::Array<Shape> shapes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    };
