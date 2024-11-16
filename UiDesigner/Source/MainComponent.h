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

class SelectionHandle
{
public:
    enum class Type
    {
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
        Rotate    // Handle for rotation
    };

    SelectionHandle();
    SelectionHandle(Type type);
    
    SelectionHandle(const SelectionHandle& other);
    SelectionHandle& operator=(const SelectionHandle& other);

    static juce::Point<float> rotatePointAround(juce::Point<float> point,
                                                juce::Point<float> center,
                                                float angleInRadians);
    
    void updatePosition(const juce::Rectangle<float>& bounds, float rotationAngle, const juce::Point<float>& rotationCenter);

    void paint(juce::Graphics& g);

    bool hitTest(juce::Point<float> point) const;

    Type getType() const { return handleType; }
    juce::Point<float> getPosition() const { return position; }
    juce::Rectangle<float> getBounds() const { return handleBounds; }

private:
    Type handleType;
    juce::Point<float> position;
    juce::Rectangle<float> handleBounds;
    juce::Rectangle<float> bounds_;  // Reference to the shape bounds
};


class MainComponent : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::ChangeListener,
                      public juce::KeyListener
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
        Style style;
        float rotation = 0.0f;  // Add rotation angle
        juce::Point<float> rotationCenter;  // Add this line
        
        bool hitTest(juce::Point<float> point) const
        {
            if (rotation != 0.0f)
            {
                // For rotated shapes, transform the test point back
                point = SelectionHandle::rotatePointAround(point, bounds.getCentre(), -rotation);
            }

            switch (type)
            {
                case Tool::Rectangle:
                    return bounds.contains(point);
                case Tool::Ellipse:
                    return bounds.reduced(style.strokeWidth * 0.5f)
                               .contains(point);
                case Tool::Line:
                {
                    float threshold = style.strokeWidth + 4.0f;
                    juce::Line<float> line(bounds.getTopLeft(), bounds.getBottomRight());
                    juce::Point<float> foundPoint;
                    return line.getDistanceFromPoint(point, foundPoint) < threshold;
                }
                    
                default:
                    return false;
            }
        }

        void initializeRotationCenter()
        {
            rotationCenter = bounds.getCentre();
        }

        void move(float dx, float dy)
        {
            bounds.translate(dx, dy);
            rotationCenter.addXY(dx, dy);  // Move the rotation center with the shape
        }
    };

    MainComponent();
    

    void paint(juce::Graphics& g) override;
     
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;

    void mouseDrag(const juce::MouseEvent& e) override;

    void mouseUp(const juce::MouseEvent& e) override;

    void sliderValueChanged(juce::Slider* slider) override;
    
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    bool keyPressed(const juce::KeyPress& key, Component *originatingComponent) override;
    bool keyStateChanged (bool isKeyDown, Component *originatingComponent) override;

private:
    void showColorPicker(bool isFillColor);

    void applyStyle(juce::Graphics& g, const Style& style);

    template<typename PathFunction>
    void drawStrokedPath(juce::Graphics& g, const Style& style, PathFunction&& pathFunc);

    // UI Components
    juce::TextButton rectangleButton;
    juce::TextButton ellipseButton;
    juce::TextButton lineButton;
    juce::TextButton selectButton;

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
    
    // Selection related members
    int selectedShapeIndex = -1;
    juce::Array<SelectionHandle> selectionHandles;
    bool isDraggingShape = false;
    bool isDraggingHandle = false;
    SelectionHandle::Type activeHandle = SelectionHandle::Type::TopLeft;
    juce::Point<float> lastMousePosition;
    float initialRotation = 0.0f;
    float initialAngle = 0.0f;

    void updateSelectionHandles();
    void handleShapeManipulation(const juce::MouseEvent& e);
    void rotateShape(const juce::MouseEvent& e);
    void resizeShape(const juce::MouseEvent& e);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    };
