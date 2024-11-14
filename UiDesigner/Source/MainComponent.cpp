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
    
    auto drawLine = [&](const juce::Point<float> start, const juce::Point<float> end, const Style style)
    {
        drawStrokedPath(g, style, [&]()
        {
            juce::Path path;
            path.startNewSubPath(start);
            path.lineTo(end);
            return path;
        });
    };
    
    
    // Draw all completed shapes
    for (const auto& shape : shapes)
    {
        applyStyle(g, shape.style);
        
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
                drawLine(shape.startPoint, shape.endPoint, shape.style);
                break;
            }
            default:
                break;
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
                drawLine(dragStart, dragEnd, currentStyle);
                break;
            }
            default:
                break;
        }
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
    rectangleButton.setBounds(padding, y, buttonWidth, buttonHeight);
    ellipseButton.setBounds(padding * 2 + buttonWidth, y, buttonWidth, buttonHeight);
    lineButton.setBounds(padding * 3 + buttonWidth * 2, y, buttonWidth, buttonHeight);
    
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
    if (e.y > 200) // Don't draw in the UI area
    {
        isDrawing = true;
        dragStart = e.position;
        dragEnd = e.position;
    }
}

void MainComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (isDrawing)
    {
        dragEnd = e.position;
        repaint();
    }
}

void MainComponent::mouseUp(const juce::MouseEvent& e)
{
    if (isDrawing)
    {
        isDrawing = false;
        
        Shape shape;
        shape.type = currentTool;
        shape.style = currentStyle;
        
        switch (currentTool)
        {
            case Tool::Rectangle:
            case Tool::Ellipse:
                shape.bounds = juce::Rectangle<float>(dragStart, dragEnd);
                break;
            case Tool::Line:
                shape.startPoint = dragStart;
                shape.endPoint = dragEnd;
                break;
            default:
                break;
        }
        
        shapes.add(shape);
        repaint();
    }
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
