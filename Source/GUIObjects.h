/*
 // Copyright (c) 2021 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#pragma once

#include <type_traits>
#include <utility>
#include <JuceHeader.h>
#include <m_pd.h>
#include "Pd/x_libpd_extra_utils.h"
#include "Pd/PdGui.hpp"
#include "Pd/PdPatch.hpp"
#include "LookAndFeel.h"
#include "PluginProcessor.h"

class Canvas;
class Box;
struct GUIComponent : public Component
{
    std::unique_ptr<ResizableBorderComponent> resizer;
    
    Box* box;
    
    GUIComponent(pd::Gui gui, Box* parent);
    
    virtual ~GUIComponent();
    
    virtual std::pair<int, int> getBestSize() = 0;
    
    virtual std::tuple<int, int, int, int> getSizeLimits() = 0;
    
    void paint(Graphics& g) override {
        g.setColour(findColour(TextButton::buttonColourId));
        g.fillRect(getLocalBounds().reduced(2));
    }
    
    void paintOverChildren(Graphics& g) override {
        g.setColour(findColour(ComboBox::outlineColourId));
        g.drawLine(0, 0, getWidth(), 0);
    }
    
    static GUIComponent* createGui(String name, Box* parent);
    
    virtual void updateValue();
    
    virtual void update() {};
    
    virtual pd::Patch* getPatch() {
        return nullptr;
    }
    
    virtual Canvas* getCanvas() {
        return nullptr;
    }

    std::unique_ptr<Label> getLabel();
    pd::Gui getGUI();
    
    float getValueOriginal() const noexcept;
    void setValueOriginal(float v, bool sendNotification = true);
    float getValueScaled() const noexcept;
    void setValueScaled(float v);
    
    void startEdition() noexcept;
    void stopEdition() noexcept;
    
protected:
    const std::string stringGui = std::string("gui");
    const std::string stringMouse = std::string("mouse");
    
    
    PlugDataAudioProcessor& processor;
    pd::Gui     gui;
    std::atomic<bool> edited;
    float       value   = 0;
    float       min     = 0;
    float       max     = 1;
    
    
    PdGuiLook guiLook;
};

struct BangComponent : public GUIComponent, public Timer
{

    
    TextButton bangButton;
    
    BangComponent(pd::Gui gui, Box* parent);
    
    std::pair<int, int> getBestSize() override {
        auto [x, y, w, h] = gui.getBounds();
        float mult = std::min(h, w) > 20.0f ? 1.0f : 2.0f;
        return {w * mult, h * mult};
    };
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        return {40, 60, 200, 200};
    };
    
    void update() override;
    
    void resized() override;
    
    void timerCallback() override {
        bangButton.setToggleState(false, dontSendNotification);
        stopTimer();
    };
    
};

struct ToggleComponent : public GUIComponent
{
    
    TextButton toggleButton;
    
    ToggleComponent(pd::Gui gui, Box* parent);
    
    std::pair<int, int> getBestSize() override {
        auto [x, y, w, h] = gui.getBounds();
        float mult = std::min(h, w) > 20.0f ? 1.0f : 2.0f;
        return {w * mult, h * mult};
        
    };
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        return {40, 60, 200, 200};
    };
    
    void resized() override;
    
    void update() override;
};


struct MessageComponent : public GUIComponent
{
    
    TextEditor input;
    TextButton bangButton;
    
    std::string lastMessage = "";
    
    MessageComponent(pd::Gui gui, Box* parent);
    
    std::pair<int, int> getBestSize() override {
        auto [x, y, w, h] = gui.getBounds();
        int offset = bangButton.isVisible() ? 60 : 30;
        int stringLength = input.getFont().getStringWidth(input.getText());
        return {stringLength + offset, numLines * 25};
        
    };
    
    
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        return {100, 50, 500, 600};
    };
    
    void updateValue() override;
    
    void resized() override;

    void update() override;
    
    int numLines = 1;
    int longestLine = 7;
};


struct NumboxComponent : public GUIComponent
{

    TextEditor input;
    
    NumboxComponent(pd::Gui gui, Box* parent);
    
    std::pair<int, int> getBestSize() override {return {50, 26}; };
    
    std::tuple<int, int, int, int> getSizeLimits() override {
        return {100, 50, 500, 600};
    };
    
    void mouseDrag(const MouseEvent & e) override {
        startEdition();
        
        input.mouseDrag(e);
        int dist = -e.getDistanceFromDragStartY();
        if(abs(dist) > 2) {
            float newval = input.getText().getFloatValue() + ((float)dist / 100.);
            input.setText(String(newval));
        }
        //onMouseDrag();
    }
    

    
    void resized() override;
    
    void update() override;

};

struct SliderComponent : public GUIComponent
{

    bool isVertical;
    
    Slider slider;
    
    SliderComponent(bool vertical, pd::Gui gui, Box* parent);
    
    std::pair<int, int> getBestSize() override {
        if(isVertical) return {35, 130};
        
        return {130, 35}; };
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        return {100, 60, 500, 600};
    };

    void resized() override;

    void update() override;

};


struct RadioComponent : public GUIComponent
{
    
    int last_state = 0;
    
    bool isVertical;
    RadioComponent(bool vertical, pd::Gui gui, Box* parent);
    
    std::array<TextButton, 8> radio_buttons;
    
    std::pair<int, int> getBestSize() override {
        if(isVertical) return {24, 163};
        
        return {161, 23};
    };
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        return {100, 40, 500, 600};
    };
    
    
    void resized() override;

    void update() override;
    
};

struct GraphicalArray : public Component, public Timer
{
public:
    GraphicalArray(PlugDataAudioProcessor* pd, pd::Array& graph);
    void paint(Graphics& g) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;
    size_t getArraySize() const noexcept;
private:
    
    void timerCallback() override;
    
    template <typename T> T clip(const T& n, const T& lower, const T& upper) {
        return std::max(std::min(n, upper), lower);
    }
    
    pd::Array               array;
    std::vector<float>      vec;
    std::vector<float>      temp;
    std::atomic<bool>       edited;
    bool                    error = false;
    const std::string stringArray = std::string("array");
    
    PlugDataAudioProcessor* pd;
};


struct ArrayComponent : public GUIComponent
{
public:
    ArrayComponent(pd::Gui gui, Box* box);
    void paint(Graphics& ) override {}
    void resized() override;
    void updateValue() override {}
    
    
    std::pair<int, int> getBestSize() override {return {200, 140}; };
    
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        
        
        return {100, 40, 500, 600};
    };
    
private:
    pd::Array      graph;
    GraphicalArray array;
};

struct GraphOnParent : public GUIComponent
{
public:
    GraphOnParent(pd::Gui gui, Box* box);
    ~GraphOnParent();
    
    void paint(Graphics& g) override;
    void resized() override;
    void updateValue() override;

    int bestW = 410;
    int bestH = 270;
    
    std::pair<int, int> getBestSize() override {
        auto [x, y, w, h] = gui.getBounds();
        return {w, h};
        
    };
    
    
    pd::Patch* getPatch() override {
        return &subpatch;
    }
    
    Canvas* getCanvas() override {
        return canvas.get();
    }
    
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        
        
        return {30, 30, 500, 600};
    };
    
    void updateCanvas();
    
private:
    
    pd::Patch subpatch;
    std::unique_ptr<Canvas> canvas;
    
};



struct Subpatch : public GUIComponent
{

    Subpatch(pd::Gui gui, Box* box);
    
    ~Subpatch();
    
    std::pair<int, int> getBestSize() override {return {0, 3}; };
    
    void resized() override {};
    void updateValue() override {};
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        return {40, 32, 100, 32};
    };
    
    
    pd::Patch* getPatch() override {
        return &subpatch;
    }
    

    
private:
    pd::Patch subpatch;
};


struct CommentComponent : public GUIComponent
{
    CommentComponent(pd::Gui gui, Box* box);
    void paint(Graphics& g) override;
    
    void updateValue() override {};
    
    std::pair<int, int> getBestSize() override {return {120, 4}; };
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        return {40, 32, 100, 32};
    };
};


// ELSE mousepad
struct MousePad : public GUIComponent
{

    typedef struct _pad{
        t_object        x_obj;
        t_glist        *x_glist;
        void           *x_proxy; // dont have this object and dont need it
        t_symbol       *x_bindname;
        int             x_x;
        int             x_y;
        int             x_w;
        int             x_h;
        int             x_sel;
        int             x_zoom;
        int             x_edit;
        unsigned char   x_color[3];
    }t_pad;
    
    MousePad(pd::Gui gui, Box* box);
    
    
    void paint(Graphics& g) override;
    
    void updateValue() override;
    
    void mouseDown(const MouseEvent& e) override;
    void mouseMove(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    
    std::pair<int, int> getBestSize() override {
        auto [x, y, w, h] = gui.getBounds();
        return {w, h};
    };
    
    std::tuple<int, int, int, int> getSizeLimits()  override {
        return {40, 32, 100, 32};
    };
    
    t_template* t_template;
    
};

struct _fielddesc
{
    char fd_type;       /* LATER consider removing this? */
    char fd_var;
    union
    {
        t_float fd_float;       /* the field is a constant float */
        t_symbol *fd_symbol;    /* the field is a constant symbol */
        t_symbol *fd_varsym;    /* the field is variable and this is the name */
    } fd_un;
    float fd_v1;        /* min and max values */
    float fd_v2;
    float fd_screen1;   /* min and max screen values */
    float fd_screen2;
    float fd_quantum;   /* quantization in value */
};

// TODO: Pd template class for drawing (using "drawcurve", "drawpolygon", etc)
struct TemplateDraw
{
    struct t_curve
    {
        t_object x_obj;
        int x_flags;    /* CLOSED, BEZ, NOMOUSERUN, NOMOUSEEDIT */
        t_fielddesc x_fillcolor;
        t_fielddesc x_outlinecolor;
        t_fielddesc x_width;
        t_fielddesc x_vis;
        int x_npoints;
        t_fielddesc *x_vec;
        t_canvas *x_canvas;
    };    
    
    static void paintOnCanvas(Graphics& g, Canvas* canvas, t_scalar* scalar, t_gobj* obj, int baseX, int baseY);

    
};
