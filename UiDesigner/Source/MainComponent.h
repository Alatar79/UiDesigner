/*
  ==============================================================================

    MainComponent.h
    Created: 11 Nov 2024 10:52:48pm
    Author:  Martin S

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class MainComponent : public juce::Component
{
public:
    enum class Tool
    {
        Rectangle,
        Ellipse,
        Line,
        Select
    };

    struct Shape
    {
        Tool type;
        juce::Rectangle<float> bounds;
        juce::Point<float> startPoint;
        juce::Point<float> endPoint;
        juce::Colour colour;
    };

    MainComponent()
    {
        addAndMakeVisible(rectangleButton);
        addAndMakeVisible(ellipseButton);
        addAndMakeVisible(lineButton);
        
        rectangleButton.setButtonText("Rectangle");
        ellipseButton.setButtonText("Ellipse");
        lineButton.setButtonText("Line");
        
        rectangleButton.onClick = [this] { currentTool = Tool::Rectangle; };
        ellipseButton.onClick = [this] { currentTool = Tool::Ellipse; };
        lineButton.onClick = [this] { currentTool = Tool::Line; };

        // Random color for each shape
        currentColour = juce::Colour::fromHSV(random.nextFloat(), 0.7f, 0.9f, 1.0f);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::white);
        
        // Draw all completed shapes
        for (const auto& shape : shapes)
        {
            g.setColour(shape.colour);
            
            switch (shape.type)
            {
                case Tool::Rectangle:
                    g.drawRect(shape.bounds);
                    break;
                case Tool::Ellipse:
                    g.drawEllipse(shape.bounds, 1.0f);
                    break;
                case Tool::Line:
                    g.drawLine(shape.startPoint.x, shape.startPoint.y,
                             shape.endPoint.x, shape.endPoint.y);
                    break;
                default:
                    break;
            }
        }
        
        // Draw current shape being created
        if (isDrawing)
        {
            g.setColour(currentColour);
            
            switch (currentTool)
            {
                case Tool::Rectangle:
                {
                    auto bounds = getBoundsFromPoints(dragStart, dragEnd);
                    g.drawRect(bounds);
                    break;
                }
                case Tool::Ellipse:
                {
                    auto bounds = getBoundsFromPoints(dragStart, dragEnd);
                    g.drawEllipse(bounds, 1.0f);
                    break;
                }
                case Tool::Line:
                    g.drawLine(dragStart.x, dragStart.y, dragEnd.x, dragEnd.y);
                    break;
                default:
                    break;
            }
        }
    }

    void resized() override
    {
        auto buttonWidth = 100;
        auto buttonHeight = 30;
        auto padding = 10;
        
        rectangleButton.setBounds(padding, padding, buttonWidth, buttonHeight);
        ellipseButton.setBounds(padding * 2 + buttonWidth, padding, buttonWidth, buttonHeight);
        lineButton.setBounds(padding * 3 + buttonWidth * 2, padding, buttonWidth, buttonHeight);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        isDrawing = true;
        dragStart = e.position;
        dragEnd = e.position;
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        dragEnd = e.position;
        repaint();
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        isDrawing = false;
        
        Shape shape;
        shape.type = currentTool;
        shape.colour = currentColour;
        
        switch (currentTool)
        {
            case Tool::Rectangle:
            case Tool::Ellipse:
                shape.bounds = getBoundsFromPoints(dragStart, dragEnd);
                break;
            case Tool::Line:
                shape.startPoint = dragStart;
                shape.endPoint = dragEnd;
                break;
            default:
                break;
        }
        
        shapes.add(shape);
        
        // Generate new random color for next shape
        currentColour = juce::Colour::fromHSV(random.nextFloat(), 0.7f, 0.9f, 1.0f);
        
        repaint();
    }

private:
    juce::Rectangle<float> getBoundsFromPoints(const juce::Point<float>& p1, const juce::Point<float>& p2)
    {
        auto x = juce::jmin(p1.x, p2.x);
        auto y = juce::jmin(p1.y, p2.y);
        auto width = std::abs(p2.x - p1.x);
        auto height = std::abs(p2.y - p1.y);
        
        return juce::Rectangle<float>(x, y, width, height);
    }

    juce::TextButton rectangleButton;
    juce::TextButton ellipseButton;
    juce::TextButton lineButton;
    
    Tool currentTool = Tool::Rectangle;
    bool isDrawing = false;
    juce::Point<float> dragStart;
    juce::Point<float> dragEnd;
    
    juce::Array<Shape> shapes;
    juce::Colour currentColour;
    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
