/*
 // Copyright (c) 2021-2022 Timothy Schoen
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "Dialogs.h"
#include "LookAndFeel.h"

#include "PluginEditor.h"
#include "PluginProcessor.h"

#include "Standalone/PlugDataWindow.h"

#include <JuceHeader.h>

#include <memory>

struct BlackoutComponent : public Component
{
    Component* parent;
    Component* dialog;
    
    std::function<void()> onClose;

    BlackoutComponent(Component* p, Component* d, std::function<void()> closeCallback = [](){}) : parent(p), dialog(d), onClose(closeCallback) {
        parent->addAndMakeVisible(this);
        //toBehind(dialog);
        setAlwaysOnTop(true);
        dialog->setAlwaysOnTop(true);
        
        resized();
    }
    
    void paint(Graphics& g) {
        g.fillAll(Colours::black.withAlpha(0.5f));
    }
    
    void resized() {
        setBounds(parent->getLocalBounds());
    }
    
    void mouseDown(const MouseEvent& e) {
        onClose();
    }
    
};

struct SaveDialog : public Component
{
    SaveDialog(Component* editor)
    {
        setSize(400, 200);
        addAndMakeVisible(savelabel);
        addAndMakeVisible(cancel);
        addAndMakeVisible(dontsave);
        addAndMakeVisible(save);


        cancel.onClick = [this]
        {
            
            MessageManager::callAsync([this]() {
                background->setVisible(false);
                cb(0);
                delete this;
            });
        };
        save.onClick = [this]
        {
            MessageManager::callAsync([this]() {
                cb(2);
                delete this;
            });
        };
        dontsave.onClick = [this]
        {
            MessageManager::callAsync([this]() {
                cb(1);
                delete this;
            });
        };
        
        
        background.reset(new BlackoutComponent(editor, this));
        
        cancel.changeWidthToFitText();
        dontsave.changeWidthToFitText();
        save.changeWidthToFitText();
        setOpaque(false);
        
    }

    void resized() override
    {
        savelabel.setBounds(20, 25, 200, 30);
        cancel.setBounds(20, 80, 80, 25);
        dontsave.setBounds(200, 80, 80, 25);
        save.setBounds(300, 80, 80, 25);
    }

    void paint(Graphics& g) override
    {
        g.setColour(findColour(ComboBox::backgroundColourId));
        g.fillRoundedRectangle(getLocalBounds().reduced(2).toFloat(), 3.0f);

        g.setColour(findColour(ComboBox::outlineColourId));
        g.drawRoundedRectangle(getLocalBounds().reduced(2).toFloat(), 3.0f, 2.0f);
    }

    std::function<void(int)> cb;

   private:
    
    std::unique_ptr<BlackoutComponent> background;
    
    Label savelabel = Label("savelabel", "Save Changes?");

    TextButton cancel = TextButton("Cancel");
    TextButton dontsave = TextButton("Don't Save");
    TextButton save = TextButton("Save");
};

struct ArrayDialog : public Component
{
    ArrayDialog(Component* editor)
    {
        setSize(400, 200);

        addAndMakeVisible(label);
        addAndMakeVisible(cancel);
        addAndMakeVisible(ok);

        cancel.onClick = [this]
        {
            MessageManager::callAsync([this](){
                background->setVisible(false);
                cb(0, "", "");
                delete this;
            });

        };
        ok.onClick = [this]
        {
            // Check if input is valid
            if (nameEditor.isEmpty())
            {
                nameEditor.setColour(TextEditor::outlineColourId, Colours::red);
                nameEditor.giveAwayKeyboardFocus();
                nameEditor.repaint();
            }
            if (sizeEditor.getText().getIntValue() < 0)
            {
                sizeEditor.setColour(TextEditor::outlineColourId, Colours::red);
                sizeEditor.giveAwayKeyboardFocus();
                sizeEditor.repaint();
            }
            if (nameEditor.getText().isNotEmpty() && sizeEditor.getText().getIntValue() >= 0)
            {
                MessageManager::callAsync([this](){
                    background->setVisible(false);
                    cb(1, nameEditor.getText(), sizeEditor.getText());
                    delete this;
                });
            }
        };

        sizeEditor.setInputRestrictions(10, "0123456789");

        cancel.changeWidthToFitText();
        ok.changeWidthToFitText();
        
        background.reset(new BlackoutComponent(editor, this, cancel.onClick));
        

        addAndMakeVisible(nameLabel);
        addAndMakeVisible(sizeLabel);

        addAndMakeVisible(nameEditor);
        addAndMakeVisible(sizeEditor);

        nameEditor.setText("array1");
        sizeEditor.setText("100");
        

        setOpaque(false);

    }

    void resized() override
    {
        label.setBounds(20, 7, 200, 30);
        cancel.setBounds(30, getHeight() - 40, 80, 25);
        ok.setBounds(getWidth() - 110, getHeight() - 40, 80, 25);

        nameEditor.setBounds(65, 45, getWidth() - 85, 25);
        sizeEditor.setBounds(65, 85, getWidth() - 85, 25);
        nameLabel.setBounds(8, 45, 52, 25);
        sizeLabel.setBounds(8, 85, 52, 25);
        
    }

    void paint(Graphics& g) override
    {
        g.setColour(findColour(ComboBox::backgroundColourId));
        g.fillRoundedRectangle(getLocalBounds().reduced(2).toFloat(), 3.0f);

        g.setColour(findColour(ComboBox::outlineColourId).darker());
        g.drawRoundedRectangle(getLocalBounds().reduced(2).toFloat(), 3.0f, 1.5f);
    }

    std::function<void(int, String, String)> cb;

   private:

    Label label = Label("savelabel", "Array Properties");

    Label nameLabel = Label("namelabel", "Name:");
    Label sizeLabel = Label("sizelabel", "Size:");

    TextEditor nameEditor;
    TextEditor sizeEditor;

    TextButton cancel = TextButton("Cancel");
    TextButton ok = TextButton("OK");
    
    std::unique_ptr<BlackoutComponent> background;
};

struct DAWAudioSettings : public Component
{
    explicit DAWAudioSettings(AudioProcessor& p) : processor(p)
    {
        addAndMakeVisible(latencySlider);
        latencySlider.setRange(0, 88200, 1);
        latencySlider.setTextValueSuffix(" Samples");
        latencySlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxRight, false, 100, 20);

        latencySlider.onValueChange = [this]() { processor.setLatencySamples(latencySlider.getValue()); };

        addAndMakeVisible(latencyLabel);
        latencyLabel.setText("Latency", dontSendNotification);
        latencyLabel.attachToComponent(&latencySlider, true);
    }

    void resized() override
    {
        latencySlider.setBounds(90, 5, getWidth() - 130, 20);
    }

    void visibilityChanged() override
    {
        latencySlider.setValue(processor.getLatencySamples());
    }

    AudioProcessor& processor;
    Label latencyLabel;
    Slider latencySlider;
};

class SearchPathComponent : public Component, public TableListBoxModel
{
   public:
    SearchPathComponent(ValueTree libraryTree) : tree(std::move(libraryTree))
    {
        table.setModel(this);
        table.setColour(ListBox::backgroundColourId, Colour(25, 25, 25));
        table.setRowHeight(30);

        // give it a border
        // table.setColour(ListBox::outlineColourId, Colours::transparentBlack);
        table.setColour(ListBox::textColourId, Colours::white);

        table.setOutlineThickness(0);

        table.deselectAllRows();

        setColour(ListBox::textColourId, Colours::white);
        setColour(ListBox::outlineColourId, Colours::white);

        table.getHeader().setStretchToFitActive(true);
        table.setHeaderHeight(0);
        table.getHeader().addColumn("Library Path", 1, 800, 50, 800, TableHeaderComponent::defaultFlags);

        addButton.setColour(ComboBox::outlineColourId, Colours::transparentBlack);
        addButton.setConnectedEdges(12);
        addButton.setName("statusbar:add");
        addButton.onClick = [this]()
        {
            openChooser.launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories,
                                    [this](const FileChooser& f)
                                    {
                                        auto result = f.getResult();
                                        if (!result.exists()) return;

                                        auto child = ValueTree("Path");
                                        child.setProperty("Path", result.getFullPathName(), nullptr);

                                        tree.appendChild(child, nullptr);

                                        loadData();
                                    });
        };

        removeButton.setColour(ComboBox::outlineColourId, Colours::transparentBlack);
        removeButton.setConnectedEdges(12);
        removeButton.setName("statusbar:add");
        removeButton.onClick = [this]() mutable
        {
            int idx = table.getSelectedRow();
            tree.removeChild(idx, nullptr);
            loadData();
        };

        resetButton.onClick = [this]()
        {
            File abstractionsDir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("PlugData").getChildFile("Abstractions");

            auto defaultPath = ValueTree("Path");
            defaultPath.setProperty("Path", abstractionsDir.getFullPathName(), nullptr);

            tree.removeAllChildren(nullptr);
            tree.appendChild(defaultPath, nullptr);
            loadData();
        };

        addButton.setColour(ComboBox::backgroundColourId, Colour(20, 20, 20));
        removeButton.setColour(ComboBox::backgroundColourId, Colour(20, 20, 20));

        addAndMakeVisible(table);
        addAndMakeVisible(addButton);
        addAndMakeVisible(removeButton);
        addAndMakeVisible(resetButton);

        loadData();
    }

    void loadData()
    {
        items.clear();

        for (auto child : tree)
        {
            items.add(child.getProperty("Path"));
        }

        table.updateContent();
        table.selectRow(items.size() - 1);
    }

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground(Graphics& g, int row, int w, int h, bool rowIsSelected) override
    {
        if (rowIsSelected)
        {
            g.setColour(findColour(Slider::thumbColourId));
        }
        else
        {
            g.setColour((row % 2) ? findColour(ComboBox::backgroundColourId) : findColour(ResizableWindow::backgroundColourId));
        }

        g.fillRect(1, 0, w - 3, h);
    }

    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        g.setColour(Colours::white);
        const String item = tree.getChild(rowNumber).getProperty("Path").toString();

        g.drawText(item, 4, 0, width - 4, height, Justification::centredLeft, true);

        // g.setColour(Colours::black.withAlpha(0.2f));
        // g.fillRect(width - 1, 0, 1, height);
    }

    int getNumRows() override
    {
        return items.size();
    }

    Component* refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, Component* existingComponentToUpdate) override
    {
        delete existingComponentToUpdate;
        return nullptr;
    }

    void resized() override
    {
        auto tableBounds = getLocalBounds();
        tableBounds.removeFromBottom(30);

        table.setBounds(tableBounds);

        const int buttonHeight = 20;
        const int y = getHeight() - (buttonHeight + 8);
        const int x = getWidth() - 8;

        resetButton.changeWidthToFitText(buttonHeight);
        resetButton.setTopRightPosition(x, y + 6);

        addButton.setBounds(10, y, 30, 30);
        removeButton.setBounds(40, y, 30, 30);
    }

   private:
    FileChooser openChooser = FileChooser("Choose path", File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory), "");

    TextButton addButton = TextButton(Icons::Add);
    TextButton removeButton = TextButton(Icons::Clear);
    TextButton resetButton = TextButton("reset to defaults");

    TableListBox table;
    ValueTree tree;
    StringArray items;
};

struct SettingsComponent : public Component
{
    SettingsComponent(AudioProcessor& processor, AudioDeviceManager* manager, const ValueTree& settingsTree)
    {
        toolbarButtons = {new TextButton(Icons::Audio), new TextButton(Icons::Search), new TextButton(Icons::Keyboard)};

        auto* editor = dynamic_cast<ApplicationCommandManager*>(processor.getActiveEditor());

        if (manager)
        {
            panels.add(new AudioDeviceSelectorComponent(*manager, 1, 2, 1, 2, true, true, true, false));
        }
        else
        {
            panels.add(new DAWAudioSettings(processor));
        }

        panels.add(new SearchPathComponent(settingsTree.getChildWithName("Paths")));
        panels.add(new KeyMappingEditorComponent(*editor->getKeyMappings(), true));

        for (int i = 0; i < toolbarButtons.size(); i++)
        {
            toolbarButtons[i]->setClickingTogglesState(true);
            toolbarButtons[i]->setRadioGroupId(0110);
            toolbarButtons[i]->setConnectedEdges(12);
            toolbarButtons[i]->setName("toolbar:settings");
            addAndMakeVisible(toolbarButtons[i]);

            addChildComponent(panels[i]);
            toolbarButtons[i]->onClick = [this, i]() mutable { showPanel(i); };
        }

        toolbarButtons[0]->setToggleState(true, sendNotification);
    }

    void showPanel(int idx)
    {
        panels[currentPanel]->setVisible(false);
        panels[idx]->setVisible(true);
        currentPanel = idx;
    }

    void paint(Graphics& g) override
    {
        auto highlightColour = Colour(0xff42a2c8).darker(0.2);

        g.setColour(findColour(ComboBox::backgroundColourId));
        g.fillRoundedRectangle(getLocalBounds().reduced(2).toFloat(), 3.0f);

        g.setColour(highlightColour);
        g.fillRect(2, 42, getWidth() - 4, 4);

        if (currentPanel > 0)
        {
            auto tableBounds = getLocalBounds();

            g.setColour(Colour(20, 20, 20));
            g.fillRect(tableBounds.removeFromBottom(40));
        }
    }

    void resized() override
    {
        int toolbarPosition = 2;
        for (auto& button : toolbarButtons)
        {
            button->setBounds(toolbarPosition, 0, 70, toolbarHeight);
            toolbarPosition += 70;
        }

        for (auto* panel : panels)
        {
            if (panel == panels.getLast())
            {
                panel->setBounds(8, toolbarHeight, getWidth() - 8, getHeight() - toolbarHeight - 8);
                continue;
            }
            panel->setBounds(2, toolbarHeight, getWidth() - 2, getHeight() - toolbarHeight - 8);
        }
    }

    
    int currentPanel = 0;
    OwnedArray<Component> panels;
    AudioDeviceManager* deviceManager = nullptr;

    int toolbarHeight = 50;

    OwnedArray<TextButton> toolbarButtons;
};

struct SettingsDialog : public Component
{
    AudioProcessor& audioProcessor;

    SettingsComponent settingsComponent;

    ComponentBoundsConstrainer constrainer;

    SettingsDialog(AudioProcessor& processor, AudioDeviceManager* manager, const ValueTree& settingsTree) : audioProcessor(processor), settingsComponent(processor, manager, settingsTree)
    {
        closeButton.reset(getLookAndFeel().createDocumentWindowButton(4));

        setCentrePosition(400, 400);
        setSize(600, 550);

        setVisible(false);
        
        addAndMakeVisible(&settingsComponent);
        addAndMakeVisible(closeButton.get());

        settingsComponent.addMouseListener(this, false);

        
        
        closeButton->onClick = [this]()
        {
           
            dynamic_cast<PlugDataAudioProcessor*>(&audioProcessor)->saveSettings();
            setVisible(false);
        };
        
        background.reset(new BlackoutComponent(processor.getActiveEditor(), this, closeButton->onClick));

        constrainer.setMinimumOnscreenAmounts(600, 400, 400, 400);
        
    }

    ~SettingsDialog() override
    {
        settingsComponent.removeMouseListener(this);
    }
    
    void visibilityChanged() override {
        background->setVisible(isVisible());
    }

    void resized() override
    {
        closeButton->setBounds(getWidth() - 31, 3, 28, 28);
        settingsComponent.setBounds(getLocalBounds());
    }

    void paintOverChildren(Graphics& g) override
    {
        // Draw window title
        g.setColour(Colours::white);
        g.drawText("Settings", 0, 0, getWidth(), 30, Justification::centred, true);

        g.setColour(findColour(ComboBox::outlineColourId).darker());
        g.drawRoundedRectangle(getLocalBounds().reduced(2).toFloat(), 3.0f, 1.5f);
    }

    std::unique_ptr<BlackoutComponent> background;
    std::unique_ptr<Button> closeButton;
};

void Dialogs::showSaveDialog(Component* centre, std::function<void(int)> callback)
{
    auto* dialog = new SaveDialog(centre);
    dialog->cb = std::move(callback);

    centre->addAndMakeVisible(dialog);

    dialog->setBounds((centre->getWidth() / 2.) - 200., 60, 400, 130);
}
void Dialogs::showArrayDialog(Component* centre, std::function<void(int, String, String)> callback)
{
    auto* dialog = new ArrayDialog(centre);
    dialog->cb = std::move(callback);

    centre->addAndMakeVisible(dialog);

    dialog->setBounds((centre->getWidth() / 2.) - 200., 60, 300, 180);
}

std::unique_ptr<Component> Dialogs::createSettingsDialog(AudioProcessor& processor, AudioDeviceManager* manager, const ValueTree& settingsTree)
{
    return std::make_unique<SettingsDialog>(processor, manager, settingsTree);
}

void Dialogs::showObjectMenu(PlugDataPluginEditor* parent, Component* target, const std::function<void(String)>& cb)
{
    PopupMenu menu;

    // Custom functions because JUCE adds "shortcut:" before some keycommands, which looks terrible!
    auto createCommandItem = [parent](const CommandID commandID, const String& displayName)
    {
        ApplicationCommandInfo info(*parent->getCommandForID(commandID));
        auto* target = parent->ApplicationCommandManager::getTargetForCommand(commandID, info);

        PopupMenu::Item i;
        i.text = displayName;
        i.itemID = (int)commandID;
        i.commandManager = parent;
        i.isEnabled = target != nullptr && (info.flags & ApplicationCommandInfo::isDisabled) == 0;

        String shortcutKey;

        for (auto& keypress : parent->getKeyMappings()->getKeyPressesAssignedToCommand(commandID))
        {
            auto key = keypress.getTextDescriptionWithIcons();

            if (shortcutKey.isNotEmpty()) shortcutKey << ", ";

            if (key.length() == 1 && key[0] < 128)
                shortcutKey << key;
            else
                shortcutKey << key;
        }

        i.shortcutKeyDescription = shortcutKey.trim();

        return i;
    };

    menu.addItem(createCommandItem(CommandIDs::NewObject, "Empty Object"));
    menu.addSeparator();
    menu.addItem(createCommandItem(CommandIDs::NewNumbox, "Number"));
    menu.addItem(createCommandItem(CommandIDs::NewMessage, "Message"));
    menu.addItem(createCommandItem(CommandIDs::NewBang, "Bang"));
    menu.addItem(createCommandItem(CommandIDs::NewToggle, "Toggle"));
    menu.addItem(createCommandItem(CommandIDs::NewSlider, "Vertical Slider"));
    menu.addItem(5, "Horizontal Slider");
    menu.addItem(8, "Vertical Radio");
    menu.addItem(7, "Horizontal Radio");

    menu.addSeparator();

    menu.addItem(createCommandItem(CommandIDs::NewFloatAtom, "Float Atom"));
    menu.addItem(10, "Symbol Atom");
    menu.addItem(16, "List Atom");

    menu.addSeparator();

    menu.addItem(11, "Array");
    menu.addItem(12, "GraphOnParent");
    menu.addItem(createCommandItem(CommandIDs::NewComment, "Comment"));
    menu.addItem(14, "Canvas");
    menu.addSeparator();
    menu.addItem(15, "Keyboard");

    auto callback = [cb](int choice)
    {
        if (choice < 1) return;

        String boxName;

        switch (choice)
        {
            case 5:
                boxName = "hsl";
                break;
            case 7:
                boxName = "hradio";
                break;
            case 8:
                boxName = "vradio";
                break;
            case 10:
                boxName = "symbolatom";
                break;
            case 16:
                boxName = "listbox";
                break;
            case 11:
                boxName = "array";
                break;
            case 12:
                boxName = "graph";
                break;
            case 14:
                boxName = "cnv";
                break;
            case 15:
                boxName = "keyboard";
                break;
            default:
                return;
        }
        cb(boxName);
    };

    menu.showMenuAsync(PopupMenu::Options().withMinimumWidth(100).withMaximumNumColumns(1).withTargetComponent(target).withParentComponent(parent), ModalCallbackFunction::create(callback));
}
