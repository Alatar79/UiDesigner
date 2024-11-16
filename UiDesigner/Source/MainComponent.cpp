/*
  ==============================================================================

    MainComponent.cpp
    Created: 11 Nov 2024 10:52:48pm
    Author:  Martin S

  ==============================================================================
*/

#include "MainComponent.h"

StrokePatternButton::StrokePatternButton(const juce::String& name) : juce::Button(name)
{
    
}
    
void StrokePatternButton::paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown)
{
    auto bounds = getLocalBounds();
    g.setColour(isButtonDown ? juce::Colours::darkgrey : (isMouseOver ? juce::Colours::lightgrey : juce::Colours::white));
    g.fillRect(bounds);
    
    g.setColour(juce::Colours::black);
    g.drawRect(bounds);
    
    // Draw pattern preview
    float dashLengths[6] = { 6.0f, 3.0f }; // Default dash pattern
    if (getName() == "Dotted")
        dashLengths[0] = dashLengths[1] = 3.0f;
    else if (getName() == "Dash-Dot")
    {
        dashLengths[0] = 9.0f;
        dashLengths[1] = 3.0f;
        dashLengths[2] = 3.0f;
        dashLengths[3] = 3.0f;
    }
    
    juce::Path path;
    path.startNewSubPath(bounds.getX() + 5, bounds.getCentreY());
    path.lineTo(bounds.getRight() - 5, bounds.getCentreY());
    
    juce::PathStrokeType strokeType(2.0f);
    if (getName() != "Solid")
    {
        strokeType.setStrokeThickness(1.0f);
        strokeType.createDashedStroke(path, path, dashLengths, getName() == "Dotted" ? 2 : 4);
    }
        
    g.strokePath(path, strokeType);
}


SelectionHandle::SelectionHandle()
{
    handleType = Type::TopLeft;
}

SelectionHandle::SelectionHandle(Type type)
{
    handleType = type;
}

SelectionHandle::SelectionHandle(const SelectionHandle& other)
    : handleType(other.handleType)
    , position(other.position)
    , handleBounds(other.handleBounds)
    , bounds_(other.bounds_)
{
}

SelectionHandle& SelectionHandle::operator=(const SelectionHandle& other)
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

juce::Point<float> SelectionHandle::rotatePointAround(juce::Point<float> point,
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

void SelectionHandle::updatePosition(const juce::Rectangle<float>& bounds, float rotationAngle, const juce::Point<float>& rotationCenter)
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
        auto rotated = rotatePointAround(position, rotationCenter, rotationAngle);
        handleBounds = juce::Rectangle<float>(handleSize, handleSize).withCentre(rotated);
    }
}

void SelectionHandle::paint(juce::Graphics& g)
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

bool SelectionHandle::hitTest(juce::Point<float> point) const
{
    return handleBounds.contains(point);
}

bool MainComponent::Shape::hitTest(juce::Point<float> point) const
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

void MainComponent::Shape::initializeRotationCenter()
{
    rotationCenter = bounds.getCentre();
}

void MainComponent::Shape::move(float dx, float dy)
{
    bounds.translate(dx, dy);
    rotationCenter.addXY(dx, dy);  // Move the rotation center with the shape
}


MainComponent::MainComponent()
{
    setName("MainComponent");
    
    // Initialize tools
    addAndMakeVisible(rectangleButton);
    addAndMakeVisible(ellipseButton);
    addAndMakeVisible(lineButton);
    
    rectangleButton.setButtonText("Rectangle");
    ellipseButton.setButtonText("Ellipse");
    lineButton.setButtonText("Line");
    
    rectangleButton.onClick = [this]
    {
        currentTool = Tool::Rectangle;
    };
    ellipseButton.onClick = [this]
    {
        currentTool = Tool::Ellipse;
    };
    lineButton.onClick = [this]
    {
        currentTool = Tool::Line;
    };

    // Initialize stroke width slider
    addAndMakeVisible(strokeWidthSlider);
    strokeWidthSlider.setRange(1.0, 20.0, 1.0);
    strokeWidthSlider.setValue(2.0);
    strokeWidthSlider.addListener(this);
    
    addAndMakeVisible(strokeWidthLabel);
    strokeWidthLabel.setText("Stroke Width:", juce::dontSendNotification);
    strokeWidthLabel.attachToComponent(&strokeWidthSlider, true);

    // Fix color button text visibility
    fillColorButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    strokeColorButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    fillColorButton.setColour(juce::TextButton::buttonColourId, currentStyle.fillColour);
    strokeColorButton.setColour(juce::TextButton::buttonColourId, currentStyle.strokeColour);

     // Fix stroke pattern button handling
     solidStrokeButton.onClick = [this]
     {
         currentStyle.strokePattern = StrokePattern::Solid;
         repaint();
     };
     dashedStrokeButton.onClick = [this]
     {
         currentStyle.strokePattern = StrokePattern::Dashed;
         repaint();
     };
     dottedStrokeButton.onClick = [this]
     {
         currentStyle.strokePattern = StrokePattern::Dotted;
         repaint();
     };
     dashDotStrokeButton.onClick = [this]
     {
         currentStyle.strokePattern = StrokePattern::DashDot;
         repaint();
     };

    strokeWidthLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    strokeWidthSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    strokeWidthSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    
    // Initialize color buttons
    addAndMakeVisible(fillColorButton);
    addAndMakeVisible(strokeColorButton);
    addAndMakeVisible(fillColorLabel);
    addAndMakeVisible(strokeColorLabel);
    
    fillColorButton.setColour(juce::TextButton::buttonColourId, currentStyle.fillColour);
    strokeColorButton.setColour(juce::TextButton::buttonColourId, currentStyle.strokeColour);
    
    fillColorButton.onClick = [this]
    {
        showColorPicker(true);
    };
    strokeColorButton.onClick = [this]
    {
        showColorPicker(false);
    };
    
    fillColorLabel.setText("Fill:", juce::dontSendNotification);
    strokeColorLabel.setText("Stroke:", juce::dontSendNotification);

    // Initialize stroke pattern buttons
    addAndMakeVisible(solidStrokeButton);
    addAndMakeVisible(dashedStrokeButton);
    addAndMakeVisible(dottedStrokeButton);
    addAndMakeVisible(dashDotStrokeButton);
    
    solidStrokeButton.setName("Solid");
    dashedStrokeButton.setName("Dashed");
    dottedStrokeButton.setName("Dotted");
    dashDotStrokeButton.setName("Dash-Dot");
    
    solidStrokeButton.setRadioGroupId(1);
    dashedStrokeButton.setRadioGroupId(1);
    dottedStrokeButton.setRadioGroupId(1);
    dashDotStrokeButton.setRadioGroupId(1);
    
    solidStrokeButton.setToggleState(true, juce::dontSendNotification);
    
    // Set initial style
    currentStyle.fillColour = juce::Colours::blue.withAlpha(0.5f);
    currentStyle.strokeColour = juce::Colours::black;
    currentStyle.strokeWidth = 2.0f;
    currentStyle.strokePattern = StrokePattern::Solid;
    currentStyle.hasFill = true;

    // Initialize fill toggle
    addAndMakeVisible(fillToggle);
    fillToggle.setButtonText("Enable Fill");
    fillToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::black);
    fillToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::black);
    fillToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::black);
    fillToggle.setToggleState(true, juce::dontSendNotification);
    fillToggle.onClick = [this]
    {
        currentStyle.hasFill = fillToggle.getToggleState();
        repaint();
    };
    
    // Initialize corner radius controls
    addAndMakeVisible(cornerRadiusSlider);
    cornerRadiusSlider.setRange(0.0, 50.0, 1.0);
    cornerRadiusSlider.setValue(0.0);
    cornerRadiusSlider.addListener(this);
    
    addAndMakeVisible(cornerRadiusLabel);
    cornerRadiusLabel.setText("Corner Radius:", juce::dontSendNotification);
    cornerRadiusLabel.attachToComponent(&cornerRadiusSlider, true);

    // Style the slider and label like your other controls
    cornerRadiusLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    cornerRadiusSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    cornerRadiusSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);

    // Add Select tool button
    addAndMakeVisible(selectButton);
    selectButton.setButtonText("Select");
    selectButton.onClick = [this]
    {
        currentTool = Tool::Select;
    };

    // Enable keyboard listening
    addKeyListener(this);
    setWantsKeyboardFocus(true);
    
}


void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
    
    auto drawRect = [&](const juce::Rectangle<float>& bounds, const Style& style)
    {
        if (style.hasFill)
        {
            if (style.cornerRadius > 0.0f)
                g.fillRoundedRectangle(bounds, style.cornerRadius);
            else
                g.fillRect(bounds);
        }
        
        drawStrokedPath(g, style, [&]()
        {
            juce::Path path;
            if (style.cornerRadius > 0.0f)
                path.addRoundedRectangle(bounds, style.cornerRadius);
            else
                path.addRectangle(bounds);
            return path;
        });
    };
    
    auto drawEllipse = [&](const juce::Rectangle<float>& bounds, const Style& style)
    {
        if (style.hasFill)
            g.fillEllipse(bounds);
        drawStrokedPath(g, style, [&]()
        {
            juce::Path path;
            path.addEllipse(bounds);
            return path;
        });
    };
    
    auto drawLine = [&](const juce::Rectangle<float>& bounds, const Style style)
    {
        drawStrokedPath(g, style, [&]()
        {
            juce::Path path;
            path.startNewSubPath(bounds.getTopLeft());
            path.lineTo(bounds.getBottomRight());
            return path;
        });
    };
    
    // Draw all completed shapes
    for (const auto& shape : shapes)
    {
        applyStyle(g, shape.style);
        
        if (shape.rotation != 0.0f)
        {
            // Apply rotation transform
            g.saveState();
            g.addTransform(juce::AffineTransform::rotation(
                shape.rotation,
                shape.rotationCenter.x,  // Use stored rotation center
                shape.rotationCenter.y));
        }
        
        switch (shape.type)
        {
            case Tool::Rectangle:
            {
                drawRect(shape.bounds, shape.style);
                break;
            }
            case Tool::Ellipse:
            {
                drawEllipse(shape.bounds, shape.style);
                break;
            }
            case Tool::Line:
            {
                drawLine(shape.bounds, shape.style);
                break;
            }
            default:
                break;
        }
        
        // Restore original transform if we rotated
        if (shape.rotation != 0.0f)
        {
            g.restoreState();
        }
    }
    
    // Draw current shape being created
    if (isDrawing)
    {
        applyStyle(g, currentStyle);
        
        auto bounds = juce::Rectangle<float>(dragStart, dragEnd);
        switch (currentTool)
        {
            case Tool::Rectangle:
            {
                drawRect(bounds, currentStyle);
                break;
            }
            case Tool::Ellipse:
            {
                drawEllipse(bounds, currentStyle);
                break;
            }
            case Tool::Line:
            {
                drawLine(bounds, currentStyle);
                break;
            }
            default:
                break;
        }
    }
    
    // Draw selection indicators
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())
    {
        auto& selectedShape = shapes.getReference(selectedShapeIndex);//shapes[selectedShapeIndex];
        
        // Draw selection outline
        g.setColour(juce::Colours::blue);
        
        if (selectedShape.rotation != 0.0f)
        {
            // For rotated shapes, we need to draw a rotated rectangle
            juce::Path path;
            path.addRectangle(selectedShape.bounds);
            g.addTransform(juce::AffineTransform::rotation(
                selectedShape.rotation,
                selectedShape.rotationCenter.x,
                selectedShape.rotationCenter.y));
            g.strokePath(path, juce::PathStrokeType(4.0f, juce::PathStrokeType::mitered));
            g.addTransform(juce::AffineTransform::rotation(
                -selectedShape.rotation,
                selectedShape.rotationCenter.x,
                selectedShape.rotationCenter.y));
        }
        else
        {
            // For non-rotated shapes, just draw the bounds
            g.drawRect(selectedShape.bounds, 2.0f);
        }

        // Draw selection handles
        for (auto& handle : selectionHandles)
            handle.paint(g);
    }
    
}
 
void MainComponent::resized()
{
    auto padding = 10;
    auto buttonWidth = 100;
    auto buttonHeight = 30;
    auto colorButtonWidth = 60;
    auto strokePatternWidth = 40;
    auto y = padding;
    
    // Tool buttons
    selectButton.setBounds(padding, y, buttonWidth, buttonHeight);
    rectangleButton.setBounds(padding * 2 + buttonWidth, y, buttonWidth, buttonHeight);
    ellipseButton.setBounds(padding * 3 + buttonWidth * 2, y, buttonWidth, buttonHeight);
    lineButton.setBounds(padding * 4 + buttonWidth * 3, y, buttonWidth, buttonHeight);
    
    // Style controls
    y += buttonHeight + padding;
    auto labelWidth = 80;
    strokeWidthSlider.setBounds(padding + labelWidth, y, 200, buttonHeight);
    
    // Add corner radius control
    y += buttonHeight + padding;
    cornerRadiusSlider.setBounds(padding + labelWidth, y, 200, buttonHeight);
    
    y += buttonHeight + padding;
    fillColorButton.setBounds(padding + labelWidth, y, colorButtonWidth, buttonHeight);
    fillColorLabel.setBounds(padding, y, labelWidth, buttonHeight);
    fillToggle.setBounds(padding + labelWidth + colorButtonWidth + padding, y, buttonWidth, buttonHeight);
    
    y += buttonHeight + padding;
    strokeColorButton.setBounds(padding + labelWidth, y, colorButtonWidth, buttonHeight);
    strokeColorLabel.setBounds(padding, y, labelWidth, buttonHeight);
    
    // Stroke pattern buttons
    y += buttonHeight + padding;
    solidStrokeButton.setBounds(padding, y, strokePatternWidth, buttonHeight);
    dashedStrokeButton.setBounds(padding * 2 + strokePatternWidth, y, strokePatternWidth, buttonHeight);
    dottedStrokeButton.setBounds(padding * 3 + strokePatternWidth * 2, y, strokePatternWidth, buttonHeight);
    dashDotStrokeButton.setBounds(padding * 4 + strokePatternWidth * 3, y, strokePatternWidth, buttonHeight);
}

void MainComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.y < 200) // Don't handle clicks in the UI area
        return;

    lastMousePosition = e.position;

    if (currentTool == Tool::Select)
    {
        // Check for handle interaction first
        for (int i = 0; i < selectionHandles.size(); ++i)
        {
            if (selectionHandles[i].hitTest(e.position))
            {
                isDraggingHandle = true;
                activeHandle = selectionHandles[i].getType();
                
                if (activeHandle == SelectionHandle::Type::Rotate)
                {
                    prepareRotation(e, shapes.getReference(selectedShapeIndex));
                }
                return;
            }
        }

        // Check for shape selection
        bool foundShape = false;
        for (int i = shapes.size() - 1; i >= 0; --i)
        {
            if (shapes[i].hitTest(e.position))
            {
                if (selectedShapeIndex != i) // Only update if selecting a different shape
                {
                    selectedShapeIndex = i;
                    updateSelectionHandles();
                }
                isDraggingShape = true;
                foundShape = true;
                break;
            }
        }

        if (!foundShape && selectedShapeIndex != -1) // Only update if deselecting
        {
            selectedShapeIndex = -1;
            updateSelectionHandles();
        }
    }
    else
    {
        isDrawing = true;
        dragStart = e.position;
        dragEnd = e.position;
    }
}
void MainComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (currentTool == Tool::Select)
    {
        handleShapeManipulation(e);
    }
    else if (isDrawing)
    {
        dragEnd = e.position;
        repaint();
    }
}

void MainComponent::mouseUp(const juce::MouseEvent& e)
{
    if (currentTool == Tool::Select)
    {
        isDraggingShape = false;
        isDraggingHandle = false;
    }
    else if (isDrawing)
    {
        isDrawing = false;
        
        Shape shape;
        shape.type = currentTool;
        shape.style = currentStyle;
        shape.bounds = juce::Rectangle<float>(dragStart, dragEnd);
        shape.initializeRotationCenter();
        
        shapes.add(shape);
        repaint();
    }
}

void MainComponent::handleShapeManipulation(const juce::MouseEvent& e)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())
    {
        auto delta = e.position - lastMousePosition;
        auto& shape = shapes.getReference(selectedShapeIndex);//shapes[selectedShapeIndex];

        if (isDraggingHandle)
        {
            if (activeHandle == SelectionHandle::Type::Rotate)
                rotateShape(e);
            else
                resizeShape(e);
        }
        else if (isDraggingShape)
        {
            // Move the shape
            shape.move(delta.x, delta.y);
            updateSelectionHandles();
        }

        repaint();
    }

    lastMousePosition = e.position;
}

void MainComponent::rotateShape(const juce::MouseEvent& e)
{
    auto& shape = shapes.getReference(selectedShapeIndex);
    
    // Calculate current angle relative to the stored rotation center
    float currentAngle = std::atan2(e.position.y - shape.rotationCenter.y,
                                    e.position.x - shape.rotationCenter.x);
    
    // Update shape rotation
    shape.rotation = initialRotation + (currentAngle - initialAngle);
    
    updateSelectionHandles();
}

void MainComponent::resizeShape(const juce::MouseEvent& e)
{
    auto& shape = shapes.getReference(selectedShapeIndex);
    auto delta = e.position - lastMousePosition;

    // Transform the delta based on rotation
    float cosAngle = std::cos(-shape.rotation);
    float sinAngle = std::sin(-shape.rotation);
    
    // Rotate the delta vector to account for shape rotation
    float transformedDeltaX = delta.x * cosAngle - delta.y * sinAngle;
    float transformedDeltaY = delta.x * sinAngle + delta.y * cosAngle;
    
    // Create transformed delta
    juce::Point<float> transformedDelta(transformedDeltaX, transformedDeltaY);

    // Handle resizing based on the active handle
    switch (activeHandle)
    {
        case SelectionHandle::Type::TopLeft:
            shape.bounds.setTop(shape.bounds.getY() + transformedDelta.y);
            shape.bounds.setLeft(shape.bounds.getX() + transformedDelta.x);
            break;
        case SelectionHandle::Type::Top:
            shape.bounds.setTop(shape.bounds.getY() + transformedDelta.y);
            break;
        case SelectionHandle::Type::TopRight:
            shape.bounds.setTop(shape.bounds.getY() + transformedDelta.y);
            shape.bounds.setRight(shape.bounds.getRight() + transformedDelta.x);
            break;
        case SelectionHandle::Type::Right:
            shape.bounds.setRight(shape.bounds.getRight() + transformedDelta.x);
            break;
        case SelectionHandle::Type::BottomRight:
            shape.bounds.setBottom(shape.bounds.getBottom() + transformedDelta.y);
            shape.bounds.setRight(shape.bounds.getRight() + transformedDelta.x);
            break;
        case SelectionHandle::Type::Bottom:
            shape.bounds.setBottom(shape.bounds.getBottom() + transformedDelta.y);
            break;
        case SelectionHandle::Type::BottomLeft:
            shape.bounds.setBottom(shape.bounds.getBottom() + transformedDelta.y);
            shape.bounds.setLeft(shape.bounds.getX() + transformedDelta.x);
            break;
        case SelectionHandle::Type::Left:
            shape.bounds.setLeft(shape.bounds.getX() + transformedDelta.x);
            break;
        default:
            break;
    }

    updateSelectionHandles();
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &strokeWidthSlider)
    {
        currentStyle.strokeWidth = (float)slider->getValue();
        repaint();
    }
    else if (slider == &cornerRadiusSlider)
    {
        currentStyle.cornerRadius = (float)slider->getValue();
        repaint();
    }
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (auto* cs = dynamic_cast<juce::ColourSelector*>(source))
    {
        if (currentlyEditingFillColour)
        {
            currentStyle.fillColour = cs->getCurrentColour();
            fillColorButton.setColour(juce::TextButton::buttonColourId, currentStyle.fillColour);
        }
        else
        {
            currentStyle.strokeColour = cs->getCurrentColour();
            strokeColorButton.setColour(juce::TextButton::buttonColourId, currentStyle.strokeColour);
        }
        repaint();
    }
}

bool MainComponent::keyPressed(const juce::KeyPress& key, Component* /*originatingComponent*/)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())
    {
        auto& shape = shapes.getReference(selectedShapeIndex);//shapes[selectedShapeIndex];
        
        if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
        {
            // Delete selected shape
            shapes.remove(selectedShapeIndex);
            selectedShapeIndex = -1;
            updateSelectionHandles();
            repaint();
            return true;
        }
        // Add shift+arrow keys for bigger movements
        else if (key.isKeyCode(juce::KeyPress::leftKey) && key.getModifiers().isShiftDown())
        {
            shape.move(-10.0f, 0.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::rightKey) && key.getModifiers().isShiftDown())
        {
            shape.move(10.0f, 0.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::upKey) && key.getModifiers().isShiftDown())
        {
            shape.move(0.0f, -10.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::downKey) && key.getModifiers().isShiftDown())
        {
            shape.move(0.0f, 10.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::leftKey))
        {
            // Move left
            shape.move(-1.0f, 0.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::rightKey))
        {
            // Move right
            shape.move(1.0f, 0.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::upKey))
        {
            // Move up
            shape.move(0.0f, -1.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::downKey))
        {
            // Move down
            shape.move(0.0f, 1.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
    }
    
    return false;
}

bool MainComponent::keyStateChanged(bool isKeyDown, Component *originatingComponent)
{
    return false; 
}

void MainComponent::prepareRotation(const juce::MouseEvent& e, MainComponent::Shape& shape)
{
    // Get the corners of the bounding rectangle
    auto topLeft = shape.bounds.getTopLeft();
    auto topRight = shape.bounds.getTopRight();
    auto bottomLeft = shape.bounds.getBottomLeft();
    auto bottomRight = shape.bounds.getBottomRight();
    
    // Rotate all corners around the current rotation center using the current rotation
    auto rotatePoint = [&shape](juce::Point<float> point) {
        return SelectionHandle::rotatePointAround(point, shape.rotationCenter, shape.rotation);
    };
    
    auto rotatedTopLeft = rotatePoint(topLeft);
    auto rotatedTopRight = rotatePoint(topRight);
    auto rotatedBottomLeft = rotatePoint(bottomLeft);
    auto rotatedBottomRight = rotatePoint(bottomRight);
    
    juce::Point<float> tempCenter = {
        (rotatedTopLeft.x + rotatedTopRight.x + rotatedBottomLeft.x + rotatedBottomRight.x) / 4.0f,
        (rotatedTopLeft.y + rotatedTopRight.y + rotatedBottomLeft.y + rotatedBottomRight.y) / 4.0f
    };
    
    auto rotatePointReverse = [&shape, tempCenter](juce::Point<float> point) {
        return SelectionHandle::rotatePointAround(point, tempCenter, -shape.rotation);
    };
    
    rotatedTopLeft = rotatePointReverse(rotatedTopLeft);
    rotatedTopRight = rotatePointReverse(rotatedTopRight);
    rotatedBottomLeft = rotatePointReverse(rotatedBottomLeft);
    rotatedBottomRight = rotatePointReverse(rotatedBottomRight);
    
    //Create new bounds from the back-rotated rectangle:
    shape.bounds = juce::Rectangle<float>(rotatedTopLeft, rotatedBottomRight);
    
    // Calculate the center of the back-rotated rectangle
    shape.rotationCenter = {
        (rotatedTopLeft.x + rotatedTopRight.x + rotatedBottomLeft.x + rotatedBottomRight.x) / 4.0f,
        (rotatedTopLeft.y + rotatedTopRight.y + rotatedBottomLeft.y + rotatedBottomRight.y) / 4.0f
    };
    
    // Store initial angle for rotation
    initialAngle = std::atan2(e.position.y - shape.rotationCenter.y,
                             e.position.x - shape.rotationCenter.x);
    initialRotation = shape.rotation;
}

void MainComponent::showColorPicker(bool isFillColor)
{
    currentlyEditingFillColour = isFillColor;
    auto colourSelector = std::make_unique<juce::ColourSelector>(juce::ColourSelector::showColourspace);
    colourSelector->setCurrentColour(isFillColor ? currentStyle.fillColour : currentStyle.strokeColour);
    colourSelector->addChangeListener(this);
    colourSelector->setSize(300, 400);

    juce::CallOutBox::launchAsynchronously(
        std::move(colourSelector),
        isFillColor ? fillColorButton.getBounds() : strokeColorButton.getBounds(),
        this
    );
}

void MainComponent::applyStyle(juce::Graphics& g, const Style& style)
{
    g.setColour(style.fillColour);
}

template<typename PathFunction>
void MainComponent::drawStrokedPath(juce::Graphics& g, const Style& style, PathFunction&& pathFunc)
{
    g.setColour(style.strokeColour);
    
    // Get the path from the lambda
    juce::Path path = pathFunc();
    
    if (style.strokePattern == StrokePattern::Solid)
    {
        juce::PathStrokeType strokeType(
            style.strokeWidth,
            juce::PathStrokeType::mitered,     // Joint style
            juce::PathStrokeType::square   // End cap style
        );
        g.strokePath(path, strokeType);
    }
    else
    {
        float dashLengths[6];
        int numDashLengths;
        
        // Scale dash lengths based on stroke width to maintain visible pattern
        float scale = juce::jmax(1.0f, style.strokeWidth * 0.5f);
        scale = 1.0f;
        
        switch (style.strokePattern)
        {
            case StrokePattern::Dashed:
                dashLengths[0] = 12.0f * scale;  // Dash length
                dashLengths[1] = 6.0f * scale;   // Gap length
                numDashLengths = 2;
                break;
                
            case StrokePattern::Dotted:
                dashLengths[0] = 2.0f * scale;   // Dot length
                dashLengths[1] = 4.0f * scale;   // Gap length
                numDashLengths = 2;
                break;
                
            case StrokePattern::DashDot:
                dashLengths[0] = 12.0f * scale;  // Dash length
                dashLengths[1] = 6.0f * scale;   // Gap length
                dashLengths[2] = 2.0f * scale;   // Dot length
                dashLengths[3] = 6.0f * scale;   // Gap length
                numDashLengths = 4;
                break;
                
            default:
                numDashLengths = 0;
                break;
        }
        
        juce::Path dashedPath;
        juce::PathStrokeType strokeType(
            style.strokeWidth * 0.5f,
            juce::PathStrokeType::mitered,     // Joint style
            juce::PathStrokeType::butt   // End cap style
        );
        
        strokeType.createDashedStroke(dashedPath, path, dashLengths, numDashLengths);
        g.strokePath(dashedPath, strokeType);
    }
}

void MainComponent::updateSelectionHandles()
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())
    {
        selectionHandles.clear();
        const auto& shape = shapes[selectedShapeIndex];

        // Add resize handles
        selectionHandles.add(SelectionHandle(SelectionHandle::Type::TopLeft));
        selectionHandles.add(SelectionHandle(SelectionHandle::Type::Top));
        selectionHandles.add(SelectionHandle(SelectionHandle::Type::TopRight));
        selectionHandles.add(SelectionHandle(SelectionHandle::Type::Right));
        selectionHandles.add(SelectionHandle(SelectionHandle::Type::BottomRight));
        selectionHandles.add(SelectionHandle(SelectionHandle::Type::Bottom));
        selectionHandles.add(SelectionHandle(SelectionHandle::Type::BottomLeft));
        selectionHandles.add(SelectionHandle(SelectionHandle::Type::Left));
        
        // Add rotation handle
        selectionHandles.add(SelectionHandle(SelectionHandle::Type::Rotate));

        // Update handle positions
        for (auto& handle : selectionHandles)
            handle.updatePosition(shape.bounds, shape.rotation, shape.rotationCenter);
    }
    else
    {
        selectionHandles.clear();
    }
    
    repaint();
}
