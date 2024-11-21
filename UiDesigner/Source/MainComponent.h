/*
  ==============================================================================

    MainComponent.h
    Created: 11 Nov 2024 10:52:48pm
    Author:  Martin S

    You may use this code under the terms of the GPL v3 (see
    www.gnu.org/licenses) or also the licensed attached to this project.

    THIS CODE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
    EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
    DISCLAIMED.
 
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

//Forward declaration
class ToolWindow;

class MainComponent : public juce::Component,
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
        float rotation = 0.0f;
        juce::Point<float> rotationCenter;
        juce::Point<float> lineStart;
        juce::Point<float> lineEnd;
        
        bool hitTest(juce::Point<float> point) const;
        void initializeRotationCenter();
        void move(float dx, float dy);
    };

    MainComponent();
    ~MainComponent() {}

    void paint(juce::Graphics& g) override;
     
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;

    void mouseDrag(const juce::MouseEvent& e) override;

    void mouseUp(const juce::MouseEvent& e) override;
    
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    bool keyPressed(const juce::KeyPress& key, Component *originatingComponent) override;
    bool keyStateChanged (bool isKeyDown, Component *originatingComponent) override;
    
    void deselectAllShapes();
    void setCurrentTool(Tool tool);
    void setFillEnabled(bool enabled);
    void setFillColour(juce::Colour colour);
    void setStrokeColour(juce::Colour colour);
    void setStrokeWidth(float width);
    void setCornerRadius(float radius);
    void setStrokePattern(StrokePattern pattern);
    
    void updateSelectedShapeFill(bool enabled);
    void updateSelectedShapeFillColour(juce::Colour colour);
    void updateSelectedShapeStrokeColour(juce::Colour colour);
    void updateSelectedShapeStrokeWidth(float width);
    void updateSelectedShapeCornerRadius(float radius);
    void updateSelectedShapeStrokePattern(StrokePattern pattern);
        
    const Shape* getSelectedShape() const;
    void updateSelectedShapeBounds(const juce::Rectangle<float>& newBounds);
    
private:
    
    void updateToolPanelFromShape(const Shape* shape);
    void drawDimensionLabel(juce::Graphics& g, const Shape& shape);
    void showTools();
    void prepareRotation(const juce::MouseEvent& e, Shape& shape);
    void applyStyle(juce::Graphics& g, const Style& style);

    template<typename PathFunction>
    void drawStrokedPath(juce::Graphics& g, const Style& style, PathFunction&& pathFunc);

    void updateSelectionHandles();
    void handleShapeManipulation(const juce::MouseEvent& e);
    void rotateShape(const juce::MouseEvent& e);
    void resizeShape(const juce::MouseEvent& e);
    
    std::unique_ptr<ToolWindow> toolWindow;
    juce::TextButton showToolsButton;
    
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

//==================================================================

class ColorPickerWindow : public juce::DocumentWindow, public juce::ChangeListener
{
public:
    ColorPickerWindow(const juce::String& name,
                      juce::Colour initialColor,
                      std::function<void(juce::Colour)> colorCallback);
    
    void closeButtonPressed() override;
    
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    
private:
    std::unique_ptr<juce::ColourSelector> colorSelector;
    std::function<void(juce::Colour)> colorCallback;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorPickerWindow)
};

class ToolPanel : public juce::Component
{
public:
    ToolPanel(MainComponent& mainComponent);
    
    ~ToolPanel();

    void resized() override;

    void updateDimensionEditors(const MainComponent::Shape* shape);
    void updateFromShape(const MainComponent::Shape* shape);
    
private:
    
    void deleteColorPickers();
    
    void showColorPicker(bool isFillColor);
    MainComponent& owner;
    
    // Keep track of our color picker windows
    std::unique_ptr<ColorPickerWindow> fillColorPicker;
    std::unique_ptr<ColorPickerWindow> strokeColorPicker;
    
    juce::TextButton selectButton;
    juce::TextButton rectangleButton;
    juce::TextButton ellipseButton;
    juce::TextButton lineButton;
    
    juce::Slider strokeWidthSlider;
    juce::Label strokeWidthLabel;
    juce::Slider cornerRadiusSlider;
    juce::Label cornerRadiusLabel;
    
    juce::TextButton fillColorButton{ "Fill Color" };
    juce::TextButton strokeColorButton{ "Stroke Color" };
    juce::Label fillColorLabel;
    juce::Label strokeColorLabel;
    
    StrokePatternButton solidStrokeButton{ "Solid" };
    StrokePatternButton dashedStrokeButton{ "Dashed" };
    StrokePatternButton dottedStrokeButton{ "Dotted" };
    StrokePatternButton dashDotStrokeButton{ "Dash-Dot" };
    
    juce::ToggleButton fillToggle;
    
    juce::Label widthLabel;
    juce::Label heightLabel;
    juce::TextEditor widthEditor;
    juce::TextEditor heightEditor;
    
    bool updatingFromShape = false;  // prevent feedback loops
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToolPanel)
};

class ToolWindow : public juce::DocumentWindow
{
public:
    ToolWindow(MainComponent& mainComponent);

    void closeButtonPressed() override;
    
    ToolPanel& getToolPanel();

private:
    std::unique_ptr<ToolPanel> toolPanel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToolWindow)
};
