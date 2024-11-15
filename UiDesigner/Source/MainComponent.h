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

    SelectionHandle()
    {
        handleType = Type::TopLeft;
    }
    
    SelectionHandle(Type type)
    {
        handleType = type;
    }
    
    SelectionHandle(const SelectionHandle& other)
        : handleType(other.handleType)
        , position(other.position)
        , handleBounds(other.handleBounds)
        , bounds_(other.bounds_)
    {
    }
    
    SelectionHandle& operator=(const SelectionHandle& other)
    {
        if (this != &other)
        {
            handleType = other.handleType;
            position = other.position;
            handleBounds = other.handleBounds;
            bounds_ = other.bounds_;
        }
        return *this;
    }

    static juce::Point<float> rotatePointAround(juce::Point<float> point,
                                                juce::Point<float> center,
                                                float angleInRadians)
    {
        float dx = point.x - center.x;
        float dy = point.y - center.y;
        
        return juce::Point<float>(
            center.x + (dx * std::cos(angleInRadians) - dy * std::sin(angleInRadians)),
            center.y + (dx * std::sin(angleInRadians) + dy * std::cos(angleInRadians))
        );
    }
    
    void updatePosition(const juce::Rectangle<float>& bounds, float rotationAngle)
    {
        const float handleSize = 8.0f;
        const float rotateHandleOffset = 20.0f;
        bounds_ = bounds;
        
        // Calculate handle position based on type
        switch (handleType)
        {
            case Type::TopLeft:     position = bounds.getTopLeft(); break;
            case Type::Top:         position = bounds.getCentre().withY(bounds.getY()); break;
            case Type::TopRight:    position = bounds.getTopRight(); break;
            case Type::Right:       position = bounds.getCentre().withX(bounds.getRight()); break;
            case Type::BottomRight: position = bounds.getBottomRight(); break;
            case Type::Bottom:      position = bounds.getCentre().withY(bounds.getBottom()); break;
            case Type::BottomLeft:  position = bounds.getBottomLeft(); break;
            case Type::Left:        position = bounds.getCentre().withX(bounds.getX()); break;
            case Type::Rotate:
                position = bounds.getCentre().withY(bounds.getY() - rotateHandleOffset);
                break;
        }

        // Create handle bounds centered on position
        handleBounds = juce::Rectangle<float>(handleSize, handleSize).withCentre(position);

        // If there's rotation, rotate the handle position around the bounds center
        if (rotationAngle != 0.0f && handleType != Type::Rotate)
        {
            auto center = bounds.getCentre();
            auto rotated = rotatePointAround(position, center, rotationAngle);
            handleBounds = juce::Rectangle<float>(handleSize, handleSize).withCentre(rotated);
        }
    }

    void paint(juce::Graphics& g)
    {
        g.setColour(juce::Colours::white);
        g.fillRect(handleBounds);
        g.setColour(juce::Colours::black);
        g.drawRect(handleBounds);

        if (handleType == Type::Rotate)
        {
            // Draw a circular handle for rotation
            g.drawEllipse(handleBounds.toFloat(), 1.0f);
            // Draw line connecting to the shape
            g.drawLine(handleBounds.getCentreX(), handleBounds.getBottom(),
                      handleBounds.getCentreX(), bounds_.getY());
        }
    }

    bool hitTest(juce::Point<float> point) const
    {
        return handleBounds.contains(point);
    }

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
        juce::Point<float> startPoint;
        juce::Point<float> endPoint;
        Style style;
        float rotation = 0.0f;  // Add rotation angle
        
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
                    float threshold = style.strokeWidth * 0.5f + 2.0f;
                    juce::Line<float> line(startPoint, endPoint);
                    juce::Point<float> foundPoint;
                    return line.getDistanceFromPoint(point, foundPoint) < threshold;
                }
                default:
                    return false;
            }
        }

        void move(float dx, float dy)
        {
            bounds.translate(dx, dy);
            startPoint.addXY(dx, dy);
            endPoint.addXY(dx, dy);
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
