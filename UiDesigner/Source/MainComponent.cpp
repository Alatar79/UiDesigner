/*
  ==============================================================================

    MainComponent.cpp
    Created: 11 Nov 2024 10:52:48pm
    Author:  Martin S
 
    You may use this code under the terms of the GPL v3 (see
    www.gnu.org/licenses) or also the licensed attached to this project.

    THIS CODE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
    EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
    DISCLAIMED.

  ==============================================================================
*/

#include "MainComponent.h"

StrokePatternButton::StrokePatternButton(const juce::String& name) : juce::Button(name)
{
    
}
    
void StrokePatternButton::paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown)
{
    auto bounds = getLocalBounds();
    g.setColour((isButtonDown || getToggleState()) ? juce::Colours::grey : (isMouseOver ? juce::Colours::lightgrey : juce::Colours::white));
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
                  
        // Draw rotation arrow icon above the handle
        float iconSize = handleBounds.getWidth() * 1.25f;
        juce::Rectangle<float> iconBounds(handleBounds.getCentreX() - iconSize/2,
                                        handleBounds.getY() - iconSize - 4,
                                        iconSize, iconSize);
        
        juce::Path arrow;
        // Draw the circular arc
        arrow.addArc(iconBounds.getX(),
                    iconBounds.getY(),
                    iconSize,  // radius
                    iconSize,  // radius
                    juce::MathConstants<float>::pi * 0.1f,   // startAngle
                    juce::MathConstants<float>::pi * 1.6f,   // endAngle
                    true);  // clockwise

        // Calculate end point for arrow
        float endAngle = juce::MathConstants<float>::pi * 1.6f;
        float endX = iconBounds.getCentreX() + std::cos(endAngle) * iconSize * 0.5f;
        float endY = iconBounds.getCentreY() + std::sin(endAngle) * iconSize * 0.5f;
        
        float dAngle = juce::MathConstants<float>::pi * 1.5f;
        float dendX = iconBounds.getCentreX() + std::cos(dAngle) * iconSize * 0.5f;
        float dendY = iconBounds.getCentreY() + std::sin(dAngle) * iconSize * 0.5f;
        
        arrow.addArrow(juce::Line<float>(
            endX, endY,
            dendX, dendY),
            1.0f,  // line thickness
            3.0f,  // arrow head width
            4.0f); // arrow head length
        
        g.strokePath(arrow, juce::PathStrokeType(2.0f));
    }
}

bool SelectionHandle::hitTest(juce::Point<float> point) const
{
    const float handleHitThreshold = 1.0f;
    return handleBounds.expanded(handleHitThreshold).contains(point);
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
            return bounds.contains(point);
                       
        case Tool::Line:
        {
            float threshold = style.strokeWidth + 4.0f;
            juce::Line<float> line(lineStart, lineEnd);
            juce::Point<float> foundPoint;
            return line.getDistanceFromPoint(point, foundPoint) < threshold;
        }
            
        case Tool::Text:
            return bounds.contains(point);
            
        default:
            return false;
    }
}

void MainComponent::Shape::initializeRotationCenter()
{
    if (type == Tool::Line) {
        // For lines, use midpoint between start and end
        rotationCenter = (lineStart + lineEnd) * 0.5f;
    } else {
        // For rectangles and ellipses, use bounds center
        rotationCenter = bounds.getCentre();
    }
}

void MainComponent::Shape::move(float dx, float dy)
{
    bounds.translate(dx, dy);
    rotationCenter.addXY(dx, dy);
    
    if (type == Tool::Line)
    {
        lineStart.addXY(dx, dy);
        lineEnd.addXY(dx, dy);
    }
}

void MainComponent::Shape::drawText(juce::Graphics& g) const
{
    // Don't apply rotation here - it's handled by the main drawing code
    g.setFont(font);
    g.setColour(style.fillColour);

    if (style.textStretchEnabled)
    {
        auto textBounds = g.getCurrentFont().getStringWidth(text);
        float scaleX = bounds.getWidth() / textBounds;
        float scaleY = bounds.getHeight() / font.getHeight();
        
        g.addTransform(juce::AffineTransform::scale(scaleX, scaleY)
                      .translated(bounds.getX(), bounds.getY()));
        g.drawText(text, 0, 0, textBounds, font.getHeight(),
                  juce::Justification::left, true);
    }
    else
    {
        g.drawText(text, bounds.toType<int>(),
                  juce::Justification::left, true);
    }
}


MainComponent::MainComponent()
{
    setName("MainComponent");
    
    // Create the tool window
    toolWindow = std::make_unique<ToolWindow>(*this);
    
    addAndMakeVisible(showToolsButton);
    showToolsButton.setButtonText("Show Tools");
    showToolsButton.onClick = [this]
    {
        showTools();
    };
    
    // Set initial style
    currentStyle.fillColour = juce::Colours::lightgrey;
    currentStyle.strokeColour = juce::Colours::black;
    currentStyle.strokeWidth = 0.0f;
    currentStyle.strokePattern = StrokePattern::Solid;
    currentStyle.hasFill = true;

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
        
        if (style.strokeWidth > 0.0f)
        {
            drawStrokedPath(g, style, [&]()
            {
                juce::Path path;
                if (style.cornerRadius > 0.0f)
                    path.addRoundedRectangle(bounds, style.cornerRadius);
                else
                    path.addRectangle(bounds);
                return path;
            });
        }
    };
    
    auto drawEllipse = [&](const juce::Rectangle<float>& bounds, const Style& style)
    {
        if (style.hasFill)
            g.fillEllipse(bounds);
        
        if (style.strokeWidth > 0.0f)
        {
            drawStrokedPath(g, style, [&]()
            {
                juce::Path path;
                path.addEllipse(bounds);
                return path;
            });
        }
    };
    
    auto drawLine = [&](const Shape& shape)
    {
        Style lineStyle = shape.style;
        lineStyle.strokeWidth = std::max(1.0f, shape.style.strokeWidth);
        
        drawStrokedPath(g, lineStyle, [&]()
        {
            juce::Path path;
            path.startNewSubPath(shape.lineStart);
            path.lineTo(shape.lineEnd);
            return path;
        });
    };
    
    // Draw all completed shapes
    for (int i = 0; i < shapes.size(); ++i)
    {
        // Skip drawing the shape that's currently being edited
        if (isEditingText && i == editingShapeIndex)
            continue;
            
        const auto& shape = shapes.getReference(i);
        applyStyle(g, shape.style);
        
        if (shape.rotation != 0.0f)
        {
            // Apply rotation transform
            g.saveState();
            g.addTransform(juce::AffineTransform::rotation(
                shape.rotation,
                shape.rotationCenter.x,
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
                drawLine(shape);
                break;
            }
            case Tool::Text:
                shape.drawText(g);
                break;
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
        
        switch (currentTool)
        {
            case Tool::Rectangle:
            {
                auto bounds = juce::Rectangle<float>(dragStart, dragEnd);
                drawRect(bounds, currentStyle);
                break;
            }
            case Tool::Ellipse:
            {
                auto bounds = juce::Rectangle<float>(dragStart, dragEnd);
                drawEllipse(bounds, currentStyle);
                break;
            }
            case Tool::Line:
            {
                Shape previewShape;
                previewShape.type = Tool::Line;
                previewShape.style = currentStyle;
                previewShape.lineStart = dragStart;
                previewShape.lineEnd = dragEnd;
                drawLine(previewShape);
                break;
            }
            default:
                break;
        }
    }
    
    // Draw selection indicators
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())
    {
        auto& selectedShape = shapes.getReference(selectedShapeIndex);
        
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
            g.strokePath(path, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered));
            g.addTransform(juce::AffineTransform::rotation(
                -selectedShape.rotation,
                selectedShape.rotationCenter.x,
                selectedShape.rotationCenter.y));
        }
        else
        {
            // For non-rotated shapes, just draw the bounds
            g.drawRect(selectedShape.bounds, 1.5f);
        }

        // Draw selection handles
        for (auto& handle : selectionHandles)
            handle.paint(g);
    }
    
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())
    {
        auto& selectedShape = shapes.getReference(selectedShapeIndex);
        drawDimensionLabel(g, selectedShape);
    }
}
 
void MainComponent::resized()
{
    
    // Position the button in the top-right corner
    const int buttonWidth = 100;
    const int buttonHeight = 30;
    const int padding = 10;
    
    showToolsButton.setBounds(padding,
                             padding,
                             buttonWidth,
                             buttonHeight);
    
}

void MainComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.getMouseDownY() < showToolsButton.getBottom() + 10)
        return;
    
    lastMousePosition = e.position;
    
    if (currentTool == Tool::Text)
    {
        // Start text editing
        if (!isEditingText)
        {
            startTextEditing(e.position);
        }
        return;
    }

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
        
        if (!foundShape && selectedShapeIndex != -1)
        {
            deselectAllShapes();
        }
        else if (foundShape)
        {
            updateToolPanelFromShape(&shapes.getReference(selectedShapeIndex));
        }
    }
    else
    {
        isDrawing = true;
        dragStart = e.position;
        dragEnd = e.position;
    }
}

void MainComponent::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (currentTool == Tool::Select)
    {
        // Check each shape for a hit
        for (int i = shapes.size() - 1; i >= 0; --i)
        {
            if (shapes[i].hitTest(e.position) && shapes[i].type == Tool::Text)
            {
                startTextEditing(e.position, &shapes.getReference(i));
                break;
            }
        }
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
        
        if (currentTool == Tool::Line)
        {
            // For lines, store actual start and end points
            shape.lineStart = dragStart;
            shape.lineEnd = dragEnd;
            
            // Create bounds that encompass the line
            float left = std::min(dragStart.x, dragEnd.x);
            float right = std::max(dragStart.x, dragEnd.x);
            float top = std::min(dragStart.y, dragEnd.y);
            float bottom = std::max(dragStart.y, dragEnd.y);
            
            // Add some padding for hit testing
            shape.bounds = juce::Rectangle<float>(left, top,
                                                right - left,
                                                bottom - top);
        }
        else
        {
            shape.bounds = juce::Rectangle<float>(dragStart, dragEnd);
        }
        
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
        auto& shape = shapes.getReference(selectedShapeIndex);

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

    if (shape.type == Tool::Text && !shape.style.textStretchEnabled)
    {
        // For text, only allow moving without resizing (Figma-like behavior)
        shape.bounds.translate(transformedDelta.x, transformedDelta.y);
    }
    else if (shape.type == Tool::Line)
    {
        // For lines, we need to update the actual line points
        switch (activeHandle)
        {
            case SelectionHandle::Type::TopLeft:
                shape.lineStart.addXY(delta.x, delta.y);
                break;
            case SelectionHandle::Type::BottomRight:
                shape.lineEnd.addXY(delta.x, delta.y);
                break;
            default:
                break;
        }
        
        // Update bounds to encompass the new line position
        float left = std::min(shape.lineStart.x, shape.lineEnd.x);
        float right = std::max(shape.lineStart.x, shape.lineEnd.x);
        float top = std::min(shape.lineStart.y, shape.lineEnd.y);
        float bottom = std::max(shape.lineStart.y, shape.lineEnd.y);
        
        shape.bounds = juce::Rectangle<float>(left, top, right - left, bottom - top);
    }
    else
    {
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
    }

    updateSelectionHandles();
    
    if (toolWindow != nullptr)
        toolWindow->getToolPanel().updateDimensionEditors(&shape);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (auto* cs = dynamic_cast<juce::ColourSelector*>(source))
    {
        if (currentlyEditingFillColour)
        {
            currentStyle.fillColour = cs->getCurrentColour();
        }
        else
        {
            currentStyle.strokeColour = cs->getCurrentColour();
        }
        repaint();
    }
}

bool MainComponent::keyPressed(const juce::KeyPress& key, Component* /*originatingComponent*/)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())
    {
        auto& shape = shapes.getReference(selectedShapeIndex);
        
        if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
        {
            shapes.remove(selectedShapeIndex);
            selectedShapeIndex = -1;
            updateSelectionHandles();
            repaint();
            return true;
        }
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
            shape.move(-1.0f, 0.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::rightKey))
        {
            shape.move(1.0f, 0.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::upKey))
        {
            shape.move(0.0f, -1.0f);
            updateSelectionHandles();
            repaint();
            return true;
        }
        else if (key.isKeyCode(juce::KeyPress::downKey))
        {
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

void MainComponent::deselectAllShapes()
{
    selectedShapeIndex = -1;
    updateSelectionHandles();
    updateToolPanelFromShape(nullptr);
}

void MainComponent::setCurrentTool(Tool tool)
{
    currentTool = tool;
    if (tool != Tool::Select)
        deselectAllShapes();
}

void MainComponent::setFillEnabled(bool enabled)
{
    currentStyle.hasFill = enabled;
    repaint();
}

void MainComponent::setFillColour(juce::Colour colour)
{
    currentStyle.fillColour = colour;
    repaint();
}

void MainComponent::setStrokeColour(juce::Colour colour)
{
    currentStyle.strokeColour = colour;
    repaint();
}

void MainComponent::setStrokeWidth(float width)
{
    if (currentTool == Tool::Line)
        width = std::max(1.0f, width);
    
    currentStyle.strokeWidth = width;
    repaint();
}

void MainComponent::setCornerRadius(float radius)
{
    currentStyle.cornerRadius = radius;
    repaint();
}

void MainComponent::setStrokePattern(StrokePattern pattern)
{
    currentStyle.strokePattern = pattern;
    repaint();
}

void MainComponent::updateSelectedShapeFill(bool enabled)
{
    if (selectedShapeIndex >= 0)
    {
        shapes.getReference(selectedShapeIndex).style.hasFill = enabled;
        repaint();
    }
    // Still update current style for new shapes
    currentStyle.hasFill = enabled;
}

void MainComponent::updateSelectedShapeFillColour(juce::Colour colour)
{
    if (selectedShapeIndex >= 0)
    {
        shapes.getReference(selectedShapeIndex).style.fillColour = colour;
        repaint();
    }
    currentStyle.fillColour = colour;
}

void MainComponent::updateSelectedShapeStrokeColour(juce::Colour colour)
{
    if (selectedShapeIndex >= 0)
    {
        shapes.getReference(selectedShapeIndex).style.strokeColour = colour;
        repaint();
    }
    currentStyle.strokeColour = colour;
}

void MainComponent::updateSelectedShapeStrokeWidth(float width)
{
    if (selectedShapeIndex >= 0)
    {
        auto& shape = shapes.getReference(selectedShapeIndex);
        if (shape.type == Tool::Line)
            width = std::max(1.0f, width);
        shape.style.strokeWidth = width;
        repaint();
    }
    currentStyle.strokeWidth = width;
}

void MainComponent::updateSelectedShapeCornerRadius(float radius)
{
    if (selectedShapeIndex >= 0)
    {
        shapes.getReference(selectedShapeIndex).style.cornerRadius = radius;
        repaint();
    }
    currentStyle.cornerRadius = radius;
}

void MainComponent::updateSelectedShapeStrokePattern(StrokePattern pattern)
{
    if (selectedShapeIndex >= 0)
    {
        shapes.getReference(selectedShapeIndex).style.strokePattern = pattern;
        repaint();
    }
    currentStyle.strokePattern = pattern;
}

void MainComponent::updateSelectedShapeFontSize(float size)
{
    if (selectedShapeIndex >= 0)
    {
        auto& shape = shapes.getReference(selectedShapeIndex);
        if (shape.type == Tool::Text)
        {
            shape.style.fontSize = size;
            shape.font.setHeight(size);
            
            // Recalculate bounds based on new font size
            auto width = shape.font.getStringWidth(shape.text);
            auto height = shape.font.getHeight();
            shape.bounds.setSize(width, height);
            
            updateSelectionHandles();
            repaint();
        }
    }
    currentStyle.fontSize = size;
}

const MainComponent::Shape* MainComponent::getSelectedShape() const
{
    return selectedShapeIndex >= 0 ? &shapes.getReference(selectedShapeIndex): nullptr;
}

void MainComponent::updateSelectedShapeBounds(const juce::Rectangle<float>& newBounds)
{
    if (selectedShapeIndex >= 0 && selectedShapeIndex < shapes.size())
    {
        shapes.getReference(selectedShapeIndex).bounds = newBounds;
        updateSelectionHandles();
        repaint();
    }
}

void MainComponent::startTextEditing(juce::Point<float> position, const Shape* existingShape)
{
    isEditingText = true;
    isEditingExistingText = (existingShape != nullptr);
    
    // Store the index of the shape being edited
    if (existingShape)
    {
        for (int i = 0; i < shapes.size(); ++i)
        {
            if (&shapes.getReference(i) == existingShape)
            {
                editingShapeIndex = i;
                break;
            }
        }
    }
    
    // Create and set up text editor
    textEditor = std::make_unique<juce::TextEditor>("textEntry");
    textEditor->setMultiLine(true, false);
    textEditor->setReturnKeyStartsNewLine(false);
    textEditor->setScrollbarsShown(false);
    
    // Set up font and style
    juce::Font editorFont;
    if (existingShape)
    {
        editorFont = existingShape->font;
        textEditor->setColour(juce::TextEditor::textColourId, existingShape->style.fillColour);
    }
    else
    {
        editorFont = juce::Font(currentStyle.fontFamily, currentStyle.fontSize, juce::Font::plain);
        textEditor->setColour(juce::TextEditor::textColourId, currentStyle.fillColour);
    }
    textEditor->setFont(editorFont);
    
    // Remove ALL possible sources of offset
    textEditor->setBorder(juce::BorderSize<int>(0));
    textEditor->setIndents(0, 0);
    textEditor->setJustification(juce::Justification::topLeft);
    textEditor->setLineSpacing(1.0f);
    
    // Make it transparent
    textEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    textEditor->setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    textEditor->setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentWhite);
    textEditor->setColour(juce::TextEditor::highlightColourId, juce::Colours::lightblue.withAlpha(0.3f));
    
    // Set position and initial size
    if (existingShape)
    {
        auto bounds = existingShape->bounds;
        textEditor->setBounds(bounds.toType<int>());
        textEditor->setText(existingShape->text, false);
        textEditor->selectAll();

        // Apply rotation if the text is rotated
        if (existingShape->rotation != 0.0f)
        {
            textEditor->setTransform(juce::AffineTransform::rotation(
                existingShape->rotation,
                existingShape->rotationCenter.x,
                existingShape->rotationCenter.y));
        }
    }
    else
    {
        // Initial size for new text
        const float initialWidth = 200.0f;
        currentEditorHeight = editorFont.getHeight();
        textEditor->setBounds(position.x, position.y, initialWidth, currentEditorHeight);
    }
    
    addAndMakeVisible(textEditor.get());
    textEditor->grabKeyboardFocus();
    
    // Add text change listener to handle auto-resize
    textEditor->onTextChange = [this]()
    {
        updateTextEditorSize();
    };
    
    textEditor->onReturnKey = [this]()
    {
        finishTextEditing();
    };
    
    textEditor->onFocusLost = [this]()
    {
        finishTextEditing();
    };
}

void MainComponent::updateTextEditorSize()
{
    if (!textEditor)
        return;
    
    // Store current transform before resizing
    auto currentTransform = textEditor->getTransform();
    
    // Get current bounds to maintain position
    auto bounds = textEditor->getBounds();
    
    // Calculate width based on actual text content using the font
    juce::String currentText = textEditor->getText();
    float textWidth = textEditor->getFont().getStringWidthFloat(currentText);
    float newWidth;
    
    if (isEditingExistingText)
    {
        newWidth = textWidth + 10.0f;
    }
    else
    {
        newWidth = std::max(200.0f, textWidth + 10.0f);
    }
    
    float textHeight = textEditor->getTextHeight();
    
    // Update editor bounds while maintaining position
    textEditor->setBounds(bounds.getX(), bounds.getY(),
                         newWidth, textHeight);
    
    // Reapply the transform after resizing
    textEditor->setTransform(currentTransform);
    
    currentEditorHeight = textHeight;

    // Update shape bounds if editing existing
    if (isEditingExistingText && editingShapeIndex >= 0)
    {
        auto& shape = shapes.getReference(editingShapeIndex);
        
        // Store rotation info
        float rotation = shape.rotation;
        juce::Point<float> rotationCenter = shape.rotationCenter;
        
        // Update bounds to match new text size
        shape.bounds.setSize(newWidth, textHeight);
        
        // Maintain rotation
        shape.rotation = rotation;
        shape.rotationCenter = rotationCenter;
        
        // Update selection handles to match new bounds
        updateSelectionHandles();
    }
    
    repaint();
}

void MainComponent::finishTextEditing()
{
    if (!isEditingText || textEditor == nullptr)
        return;
        
    juce::String newText = textEditor->getText();
    if (newText.isNotEmpty())
    {
        if (isEditingExistingText && editingShapeIndex >= 0)
        {
            // Update existing shape
            auto& existingShape = shapes.getReference(editingShapeIndex);
            
            // Store rotation info
            float rotation = existingShape.rotation;
            juce::Point<float> rotationCenter = existingShape.rotationCenter;
            
            // Update text and recalculate bounds
            existingShape.text = newText;
            float width = existingShape.font.getStringWidthFloat(newText);
            float height = existingShape.font.getHeight();
            
            existingShape.bounds.setSize(width, height);
            
            // Restore rotation
            existingShape.rotation = rotation;
            existingShape.rotationCenter = rotationCenter;
            
            updateSelectionHandles();
        }
        else
        {
            // Create new shape (existing code for new text creation)
            Shape textShape;
            textShape.type = Tool::Text;
            textShape.text = newText;
            textShape.style = currentStyle;
            textShape.font = textEditor->getFont();
            
            auto width = textShape.font.getStringWidthFloat(newText);
            float ascent = textShape.font.getAscent();
            float descent = textShape.font.getDescent();
            float height = ascent + descent;
            
            auto editorBounds = textEditor->getBounds().toFloat();
            textShape.bounds = juce::Rectangle<float>(
                editorBounds.getX(),
                editorBounds.getY(),
                width,
                height);
                
            textShape.initializeRotationCenter();
            shapes.add(textShape);
        }
    }
    
    removeChildComponent(textEditor.get());
    textEditor = nullptr;
    isEditingText = false;
    isEditingExistingText = false;  // Reset the flag
    editingShapeIndex = -1;  // Reset the editing index
    repaint();
}

void MainComponent::updateToolPanelFromShape(const Shape* shape)
{
    if (toolWindow != nullptr)
    {
        auto& toolPanel = toolWindow->getToolPanel();
        toolPanel.updateFromShape(shape);
    }
}

void MainComponent::drawDimensionLabel(juce::Graphics& g, const Shape& shape)
{
    // Don't show dimensions for lines
    if (shape.type == Tool::Line)
        return;

    // Format dimension text
    juce::String dimensionText;
    int width = static_cast<int>(std::abs(shape.bounds.getWidth()));
    int height = static_cast<int>(std::abs(shape.bounds.getHeight()));
    dimensionText = juce::String(width) + " × " + juce::String(height);

    // Set up text appearance
    g.setFont(14.0f);

    // Position text below the shape
    float centreX = shape.bounds.getCentreX();
    float textY = shape.bounds.getBottom() + 15.0f;

    // Draw with a light background for better visibility
    float textWidth = g.getCurrentFont().getStringWidthFloat(dimensionText) * 1.3f;
    float textHeight = g.getCurrentFont().getHeight() * 1.3f;
    juce::Rectangle<float> textBounds(centreX - textWidth / 2, textY, textWidth, textHeight);
    
    // Save the current graphics state before rotation
    juce::Graphics::ScopedSaveState stateSave(g);
    
    // Calculate the adjusted rotation angle to keep text readable
    float adjustedRotation = shape.rotation;
    
    // Normalize the rotation angle to be between -180 and 180 degrees
    while (adjustedRotation > juce::MathConstants<float>::pi)
        adjustedRotation -= 2.0f * juce::MathConstants<float>::pi;
    while (adjustedRotation < -juce::MathConstants<float>::pi)
        adjustedRotation += 2.0f * juce::MathConstants<float>::pi;
    
    bool flipText = false;
    // If rotation is beyond ±90 degrees, flip the text by 180 degrees
    if (adjustedRotation > juce::MathConstants<float>::halfPi ||
        adjustedRotation < -juce::MathConstants<float>::halfPi)
    {
        flipText = true;
    }
    
    // Apply the same rotation as the shape
    if (shape.rotation != 0.0f)
    {
        g.addTransform(juce::AffineTransform::rotation(
            shape.rotation,
            shape.rotationCenter.x,
            shape.rotationCenter.y));
    }
    
    //Check if we need to flip the text, to make sure its not upside-down.
    if (flipText)
    {
        g.addTransform(juce::AffineTransform::rotation(
           juce::MathConstants<float>::pi,
           textBounds.getCentreX(),
           textBounds.getCentreY()));
    }
    
    g.setColour(juce::Colours::blue);
    g.fillRect(textBounds);
    
    g.setColour(juce::Colours::white);
    g.drawText(dimensionText, textBounds, juce::Justification::centred, false);
}

void MainComponent::showTools()
{
    if (toolWindow != nullptr)
    {
        toolWindow->setVisible(true);
        toolWindow->toFront(true);
    }
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

        if (shape.type == Tool::Line)
        {
            // For lines, only show handles at the endpoints
            selectionHandles.add(SelectionHandle(SelectionHandle::Type::TopLeft));
            selectionHandles.add(SelectionHandle(SelectionHandle::Type::BottomRight));
            
            // Update handle positions for line endpoints
            for (auto& handle : selectionHandles)
            {
                if (handle.getType() == SelectionHandle::Type::TopLeft)
                {
                    handle.updatePosition(juce::Rectangle<float>(shape.lineStart.x - 4,
                                                               shape.lineStart.y - 4,
                                                               8, 8),
                                        shape.rotation,
                                        shape.rotationCenter);
                }
                else // BottomRight
                {
                    handle.updatePosition(juce::Rectangle<float>(shape.lineEnd.x - 4,
                                                               shape.lineEnd.y - 4,
                                                               8, 8),
                                        shape.rotation,
                                        shape.rotationCenter);
                }
            }
        }
        else
        {
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
    }
    else
    {
        selectionHandles.clear();
    }
    
    repaint();
}

//==================================================================

ColorPickerWindow::ColorPickerWindow(const juce::String& name,
                                     juce::Colour initialColor,
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

void ColorPickerWindow::closeButtonPressed()
{
    setVisible(false);
}

void ColorPickerWindow::changeListenerCallback(juce::ChangeBroadcaster*)
{
    if (colorCallback)
        colorCallback(colorSelector->getCurrentColour());
}


//-----------------------------------

ToolPanel::ToolPanel(MainComponent& mainComponent) : owner(mainComponent)
{
 
    auto setupButton = [this](juce::TextButton& b, const juce::String& text)
    {
        b.setButtonText(text);
        b.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        b.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        b.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        b.setColour(juce::TextButton::buttonOnColourId, juce::Colours::grey);
        addAndMakeVisible(b);

    };
    
    auto setupShapeButton = [this, setupButton](juce::TextButton& b, const juce::String& text)
    {
        setupButton(b, text);
        b.setClickingTogglesState(true);
        b.setRadioGroupId(2);
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

    auto addLabel = [this](juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colours::black);
        addAndMakeVisible(label);
    };
    
    strokeWidthSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    addAndMakeVisible(strokeWidthSlider);
    cornerRadiusSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    addAndMakeVisible(cornerRadiusSlider);


    fillToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::black);
    fillToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::black);
    fillToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::black);
    addAndMakeVisible(fillToggle);

    
    addAndMakeVisible(solidStrokeButton);
    addAndMakeVisible(dashedStrokeButton);
    addAndMakeVisible(dottedStrokeButton);
    addAndMakeVisible(dashDotStrokeButton);
    
    setupButton(fillColorButton, "Fill Color");
    setupButton(strokeColorButton, "Stroke Color");
    
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
    addLabel(strokeWidthLabel, "Stroke Width");
    strokeWidthLabel.attachToComponent(&strokeWidthSlider, true);

    addLabel(cornerRadiusLabel, "Corner Radius");
    cornerRadiusLabel.attachToComponent(&cornerRadiusSlider, true);
    
    addLabel(fillColorLabel, "Fill Color");
    addLabel(strokeColorLabel, "Stroke Color");
    
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
    
    addLabel(widthLabel, "Width");
    addLabel(heightLabel, "Height");
    addAndMakeVisible(widthEditor);
    addAndMakeVisible(heightEditor);

    widthEditor.setInputRestrictions(6, "0123456789");
    heightEditor.setInputRestrictions(6, "0123456789");

    widthEditor.onReturnKey = [this]()
    {
        if (auto* shape = owner.getSelectedShape())
        {
            auto bounds = shape->bounds;
            bounds.setWidth(widthEditor.getText().getFloatValue());
            owner.updateSelectedShapeBounds(bounds);
        }
    };

    heightEditor.onReturnKey = [this]()
    {
        if (auto* shape = owner.getSelectedShape())
        {
            auto bounds = shape->bounds;
            bounds.setHeight(heightEditor.getText().getFloatValue());
            owner.updateSelectedShapeBounds(bounds);
        }
    };
    
    setupShapeButton(textButton, "Text");
    textButton.onClick = [this]
    {
        // Set default black color for text
        owner.setFillColour(juce::Colours::black);
        owner.setCurrentTool(MainComponent::Tool::Text);
        
        // Update the fill color button to show black
        fillColorButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    };
    
    // Add text controls
    fontSizeSlider.setRange(8.0, 72.0, 1.0);
    fontSizeSlider.setValue(14.0);
    fontSizeSlider.onValueChange = [this]
    {
        if (!updatingFromShape)
            owner.updateSelectedShapeFontSize((float)fontSizeSlider.getValue());
    };
    addAndMakeVisible(fontSizeSlider);
    
}

ToolPanel::~ToolPanel()
{
    // Make sure we clean up any open color picker windows
    deleteColorPickers();
}

void ToolPanel::resized()
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
    textButton.setBounds(padding, y + (buttonHeight + padding) * 4, buttonWidth, buttonHeight);
    
    y = y + (buttonHeight + padding) * 5;
    
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
    // Add font size control
    fontSizeLabel.setBounds(padding, y, labelWidth, buttonHeight);
    fontSizeSlider.setBounds(padding + labelWidth, y, 200, buttonHeight);
    
    y += buttonHeight + padding;
    widthLabel.setBounds(padding, y, labelWidth, buttonHeight);
    widthEditor.setBounds(padding + labelWidth, y, 60, buttonHeight);
    
    y += buttonHeight + padding;
    heightLabel.setBounds(padding, y, labelWidth, buttonHeight);
    heightEditor.setBounds(padding + labelWidth, y, 60, buttonHeight);
}

void ToolPanel::updateDimensionEditors(const MainComponent::Shape* shape)
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

void ToolPanel::updateFromShape(const MainComponent::Shape* shape)
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
        
        // Update text-specific controls
        if (shape->type == MainComponent::Tool::Text)
            fontSizeSlider.setValue(shape->style.fontSize, juce::dontSendNotification);
    }
    else
    {
        updateDimensionEditors(nullptr);
    }
    
    updatingFromShape = false;
}

void ToolPanel::deleteColorPickers()
{
    fillColorPicker = nullptr;
    strokeColorPicker = nullptr;
}

void ToolPanel::showColorPicker(bool isFillColor)
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


//-------------------------------------------

ToolWindow::ToolWindow(MainComponent& mainComponent)
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

void ToolWindow::closeButtonPressed()
{
    // Just hide the window when the user clicks the close button
    setVisible(false);
}

ToolPanel& ToolWindow::getToolPanel()
{
    return *toolPanel;
}

