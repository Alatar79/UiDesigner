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

class ColorPickerWindow : public juce::DocumentWindow, public juce::ChangeListener
{
public:
    ColorPickerWindow(const juce::String& name, juce::Colour initialColor,
                     std::function<void(juce::Colour)> colorCallback)
        : DocumentWindow(name, juce::Colours::lightgrey,
                        DocumentWindow::closeButton | DocumentWindow::minimiseButton)
    {
        colorSelector = std::make_unique<juce::ColourSelector>(
            juce::ColourSelector::showColourspace |
            juce::ColourSelector::showAlphaChannel |
            juce::ColourSelector::showColourAtTop |
            juce::ColourSelector::editableColour);
        colorSelector->setCurrentColour(initialColor);
        colorSelector->setSize(300, 400);
        
        colorSelector->addChangeListener(this);
        this->colorCallback = std::move(colorCallback);
        
        setContentNonOwned(colorSelector.get(), true);
        setUsingNativeTitleBar(true);
        setResizable(true, false);
        setAlwaysOnTop(true);
        
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
        toFront(true);
    }
    
    void closeButtonPressed() override
    {
        setVisible(false);
        //delete this;
    }
    
    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        if (colorCallback)
            colorCallback(colorSelector->getCurrentColour());
    }
    
private:
    std::unique_ptr<juce::ColourSelector> colorSelector;
    std::function<void(juce::Colour)> colorCallback;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorPickerWindow)
};

class ToolPanel : public juce::Component
{
public:
    ToolPanel(MainComponent& mainComponent) : owner(mainComponent)
    {
        
        auto setupShapeButton = [this](juce::TextButton& b, const juce::String& text)
        {
            b.setButtonText(text);
            b.setClickingTogglesState(true);
            addAndMakeVisible(b);
            b.setRadioGroupId(2);
            b.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
            b.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
            b.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
            b.setColour(juce::TextButton::buttonOnColourId, juce::Colours::grey);
        };
        
        setupShapeButton(selectButton, "Select");
        setupShapeButton(rectangleButton, "Rectangle");
        setupShapeButton(ellipseButton, "Ellipse");
        setupShapeButton(lineButton, "Line");
        
        selectButton.onClick = [this]
        {
            owner.setCurrentTool(MainComponent::Tool::Select);
        };
        rectangleButton.onClick = [this]
        {
            owner.setCurrentTool(MainComponent::Tool::Rectangle);
        };
        ellipseButton.onClick = [this]
        {
            owner.setCurrentTool(MainComponent::Tool::Ellipse);
        };
        lineButton.onClick = [this]
        {
            owner.setCurrentTool(MainComponent::Tool::Line);
        };

        // Move all other controls (stroke width, colors, etc.)
        addAndMakeVisible(strokeWidthSlider);
        addAndMakeVisible(strokeWidthLabel);
        addAndMakeVisible(cornerRadiusSlider);
        addAndMakeVisible(cornerRadiusLabel);
        addAndMakeVisible(fillColorButton);
        addAndMakeVisible(strokeColorButton);
        addAndMakeVisible(fillColorLabel);
        addAndMakeVisible(strokeColorLabel);
        addAndMakeVisible(fillToggle);
        addAndMakeVisible(solidStrokeButton);
        addAndMakeVisible(dashedStrokeButton);
        addAndMakeVisible(dottedStrokeButton);
        addAndMakeVisible(dashDotStrokeButton);
        
        fillColorButton.setComponentID("fillColorButton");
        strokeColorButton.setComponentID("strokeColorButton");
        
        // Set up slider listeners
        strokeWidthSlider.onValueChange = [this]
        {
            if (!updatingFromShape)
                owner.updateSelectedShapeStrokeWidth((float)strokeWidthSlider.getValue());
        };

        cornerRadiusSlider.onValueChange = [this]
        {
            if (!updatingFromShape)
                owner.updateSelectedShapeCornerRadius((float)cornerRadiusSlider.getValue());
        };
        
        // stroke pattern button callbacks:
        solidStrokeButton.onClick = [this]
        {
            if (!updatingFromShape)
                owner.updateSelectedShapeStrokePattern(MainComponent::StrokePattern::Solid);
        };

        dashedStrokeButton.onClick = [this]
        {
            if (!updatingFromShape)
                owner.updateSelectedShapeStrokePattern(MainComponent::StrokePattern::Dashed);
        };

        dottedStrokeButton.onClick = [this]
        {
            if (!updatingFromShape)
                owner.updateSelectedShapeStrokePattern(MainComponent::StrokePattern::Dotted);
        };

        dashDotStrokeButton.onClick = [this]
        {
            if (!updatingFromShape)
                owner.updateSelectedShapeStrokePattern(MainComponent::StrokePattern::DashDot);
        };


        // Set up slider properties
        strokeWidthSlider.setRange(0.0, 20.0, 1.0);
        strokeWidthSlider.setValue(0.0);
        //strokeWidthSlider.addListener(&owner);
        
        cornerRadiusSlider.setRange(0.0, 50.0, 1.0);
        cornerRadiusSlider.setValue(0.0);
        //cornerRadiusSlider.addListener(&owner);
        
        // Labels
        strokeWidthLabel.setText("Stroke Width:", juce::dontSendNotification);
        strokeWidthLabel.attachToComponent(&strokeWidthSlider, true);
        
        cornerRadiusLabel.setText("Corner Radius:", juce::dontSendNotification);
        cornerRadiusLabel.attachToComponent(&cornerRadiusSlider, true);
        
        // Color buttons
        fillColorButton.onClick = [this]
        {
            showColorPicker(true);
        };
        strokeColorButton.onClick = [this]
        {
            showColorPicker(false);
        };
        
        // Fill toggle
        fillToggle.setButtonText("Enable Fill");
        fillToggle.setToggleState(true, juce::dontSendNotification);
        fillToggle.onClick = [this]
        {
            if (!updatingFromShape)
                owner.updateSelectedShapeFill(fillToggle.getToggleState());
        };

        // Stroke pattern buttons setup
        solidStrokeButton.setRadioGroupId(1);
        solidStrokeButton.setClickingTogglesState(true);
        dashedStrokeButton.setRadioGroupId(1);
        dashedStrokeButton.setClickingTogglesState(true);
        dottedStrokeButton.setRadioGroupId(1);
        dottedStrokeButton.setClickingTogglesState(true);
        dashDotStrokeButton.setRadioGroupId(1);
        dashDotStrokeButton.setClickingTogglesState(true);

        solidStrokeButton.setToggleState(true, juce::dontSendNotification);
        
        addAndMakeVisible(widthLabel);
        addAndMakeVisible(heightLabel);
        addAndMakeVisible(widthEditor);
        addAndMakeVisible(heightEditor);

        widthLabel.setText("Width:", juce::dontSendNotification);
        heightLabel.setText("Height:", juce::dontSendNotification);

        widthEditor.setInputRestrictions(6, "0123456789");
        heightEditor.setInputRestrictions(6, "0123456789");

        widthEditor.onReturnKey = [this]() {
            if (auto* shape = owner.getSelectedShape()) {
                auto bounds = shape->bounds;
                bounds.setWidth(widthEditor.getText().getFloatValue());
                owner.updateSelectedShapeBounds(bounds);
            }
        };

        heightEditor.onReturnKey = [this]() {
            if (auto* shape = owner.getSelectedShape()) {
                auto bounds = shape->bounds;
                bounds.setHeight(heightEditor.getText().getFloatValue());
                owner.updateSelectedShapeBounds(bounds);
            }
        };
        
    }
    
    ~ToolPanel()
    {
        // Make sure we clean up any open color picker windows
        deleteColorPickers();
    }

    void resized() override
    {
        // Move the layout code from MainComponent::resized() to here
        auto padding = 10;
        auto buttonWidth = 100;
        auto buttonHeight = 30;
        auto colorButtonWidth = 60;
        auto strokePatternWidth = 40;
        auto y = padding;
        
        // Tool buttons
        selectButton.setBounds(padding, y, buttonWidth, buttonHeight);
        rectangleButton.setBounds(padding, y + buttonHeight + padding, buttonWidth, buttonHeight);
        ellipseButton.setBounds(padding, y + (buttonHeight + padding) * 2, buttonWidth, buttonHeight);
        lineButton.setBounds(padding, y + (buttonHeight + padding) * 3, buttonWidth, buttonHeight);
        
        y = y + (buttonHeight + padding) * 4;
        
        // Style controls
        auto labelWidth = 80;
        strokeWidthSlider.setBounds(padding + labelWidth, y, 200, buttonHeight);
        
        y += buttonHeight + padding;
        cornerRadiusSlider.setBounds(padding + labelWidth, y, 200, buttonHeight);
        
        y += buttonHeight + padding;
        fillColorButton.setBounds(padding + labelWidth, y, colorButtonWidth, buttonHeight);
        fillColorLabel.setBounds(padding, y, labelWidth, buttonHeight);
        fillToggle.setBounds(padding + labelWidth + colorButtonWidth + padding, y, buttonWidth, buttonHeight);
        
        y += buttonHeight + padding;
        strokeColorButton.setBounds(padding + labelWidth, y, colorButtonWidth, buttonHeight);
        strokeColorLabel.setBounds(padding, y, labelWidth, buttonHeight);
        
        y += buttonHeight + padding;
        solidStrokeButton.setBounds(padding, y, strokePatternWidth, buttonHeight);
        dashedStrokeButton.setBounds(padding * 2 + strokePatternWidth, y, strokePatternWidth, buttonHeight);
        dottedStrokeButton.setBounds(padding * 3 + strokePatternWidth * 2, y, strokePatternWidth, buttonHeight);
        dashDotStrokeButton.setBounds(padding * 4 + strokePatternWidth * 3, y, strokePatternWidth, buttonHeight);
        
        y += buttonHeight + padding;
        widthLabel.setBounds(padding, y, labelWidth, buttonHeight);
        widthEditor.setBounds(padding + labelWidth, y, 60, buttonHeight);
        
        y += buttonHeight + padding;
        heightLabel.setBounds(padding, y, labelWidth, buttonHeight);
        heightEditor.setBounds(padding + labelWidth, y, 60, buttonHeight);
    }

    void updateDimensionEditors(const MainComponent::Shape* shape)
    {
        if (shape && shape->type != MainComponent::Tool::Line)
        {
            widthEditor.setText(juce::String(static_cast<int>(std::abs(shape->bounds.getWidth()))), false);
            heightEditor.setText(juce::String(static_cast<int>(std::abs(shape->bounds.getHeight()))), false);
            widthEditor.setEnabled(true);
            heightEditor.setEnabled(true);
        }
        else
        {
            widthEditor.setText("--");
            heightEditor.setText("--");
            widthEditor.setEnabled(false);
            heightEditor.setEnabled(false);
        }
    }
    
    void updateFromShape(const MainComponent::Shape* shape)
    {
        updatingFromShape = true;
        
        if (shape)
        {
            // Update all controls to reflect the selected shape's properties
            fillToggle.setToggleState(shape->style.hasFill, juce::dontSendNotification);
            strokeWidthSlider.setValue(shape->style.strokeWidth, juce::dontSendNotification);
            cornerRadiusSlider.setValue(shape->style.cornerRadius, juce::dontSendNotification);
            
            fillColorButton.setColour(juce::TextButton::buttonColourId, shape->style.fillColour);
            strokeColorButton.setColour(juce::TextButton::buttonColourId, shape->style.strokeColour);
            
            // Update stroke pattern buttons
            solidStrokeButton.setToggleState(shape->style.strokePattern == MainComponent::StrokePattern::Solid, juce::dontSendNotification);
            dashedStrokeButton.setToggleState(shape->style.strokePattern == MainComponent::StrokePattern::Dashed, juce::dontSendNotification);
            dottedStrokeButton.setToggleState(shape->style.strokePattern == MainComponent::StrokePattern::Dotted, juce::dontSendNotification);
            dashDotStrokeButton.setToggleState(shape->style.strokePattern == MainComponent::StrokePattern::DashDot, juce::dontSendNotification);
            
            // Update dimension editors
            updateDimensionEditors(shape);
        }
        else
        {
            updateDimensionEditors(nullptr);
        }
        
        updatingFromShape = false;
    }
    
private:
    
    void deleteColorPickers()
    {
        fillColorPicker = nullptr;
        strokeColorPicker = nullptr;
    }
    
    void showColorPicker(bool isFillColor)
    {
        // If there's already a picker of this type, just focus it instead of creating a new one
        if (isFillColor && fillColorPicker != nullptr)
        {
            fillColorPicker->setVisible(true);
            fillColorPicker->toFront(true);
            return;
        }
        else if (!isFillColor && strokeColorPicker != nullptr)
        {
            strokeColorPicker->setVisible(true);
            strokeColorPicker->toFront(true);
            return;
        }

        juce::String windowTitle = isFillColor ? "Fill Color" : "Stroke Color";
        juce::Colour initialColor = isFillColor ?
            fillColorButton.findColour(juce::TextButton::buttonColourId) :
            strokeColorButton.findColour(juce::TextButton::buttonColourId);
            
        auto& picker = isFillColor ? fillColorPicker : strokeColorPicker;
        picker = std::make_unique<ColorPickerWindow>(windowTitle, initialColor,
            [this, isFillColor](juce::Colour newColour)
            {
                if (isFillColor)
                {
                    fillColorButton.setColour(juce::TextButton::buttonColourId, newColour);
                    owner.updateSelectedShapeFillColour(newColour);
                }
                else
                {
                    strokeColorButton.setColour(juce::TextButton::buttonColourId, newColour);
                    owner.updateSelectedShapeStrokeColour(newColour);
                }
            });

        // Position the picker window relative to the button
        auto &button = isFillColor ? fillColorButton : strokeColorButton;
        auto buttonPos = button.getBounds().getTopRight();
        auto screenPos = localPointToGlobal(buttonPos);
        
        // Offset it slightly so it doesn't cover the button
        screenPos.addXY(10, 0);
        
        picker->setTopLeftPosition(screenPos.x, screenPos.y);
    }
    
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
    ToolWindow(MainComponent& mainComponent)
        : DocumentWindow("Tools", juce::Colours::lightgrey,
                        DocumentWindow::closeButton | DocumentWindow::minimiseButton)
    {
        toolPanel = std::make_unique<ToolPanel>(mainComponent);
        
        const int width = 600;
        const int height = 600;
        
        // Give the tool panel an initial size before setting it as content
        toolPanel->setSize(width, height);
        
        // Use setContentNonOwned since we're managing ownership with unique_ptr
        setContentNonOwned(toolPanel.get(), true);
        
        setUsingNativeTitleBar(true);
        setResizable(true, false);
        centreWithSize(width, height);
        setVisible(true);
        //Need to make the process the foreground, otherwise the tooltip window wont show up initially, on Mac at least.
        juce::Process::makeForegroundProcess();
        toFront(true);  // Bring window to front when created
    }

    void closeButtonPressed() override
    {
        // Just hide the window when the user clicks the close button
        setVisible(false);
    }
    
    ToolPanel& getToolPanel()
    {
        return *toolPanel;
    }

private:
    std::unique_ptr<ToolPanel> toolPanel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToolWindow)
};
