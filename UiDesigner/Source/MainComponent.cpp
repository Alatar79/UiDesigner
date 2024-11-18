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
            return bounds.reduced(style.strokeWidth * 0.5f)
                       .contains(point);
                       
        case Tool::Line:
        {
            float threshold = style.strokeWidth + 4.0f;
            juce::Line<float> line(lineStart, lineEnd);
            juce::Point<float> foundPoint;
            return line.getDistanceFromPoint(point, foundPoint) < threshold;
        }
            
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


MainComponent::MainComponent()
{
    setName("MainComponent");
    
    // Create the tool window
    toolWindow = std::make_unique<ToolWindow>(*this);
    
    // Set initial style
    currentStyle.fillColour = juce::Colours::blue.withAlpha(0.5f);
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
    for (const auto& shape : shapes)
    {
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
}

void MainComponent::mouseDown(const juce::MouseEvent& e)
{
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

    if (shape.type == Tool::Line)
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

void MainComponent::setCurrentTool(Tool tool)
{
    currentTool = tool;
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
            // Add resize handles for rectangles and ellipses
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
