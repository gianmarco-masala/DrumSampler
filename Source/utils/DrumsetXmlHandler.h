#pragma once

#include <JuceHeader.h>

class DrumsetXmlHandler
{
public:
    DrumsetXmlHandler()
    {
        auto dir = file.getCurrentWorkingDirectory();
        auto path = dir.getChildFile("../../../../../Source/utils/mixer_info.xml");

        if (path.existsAsFile()) {
            auto drumsetInfo = new XmlDocument(path);
            auto mixer = drumsetInfo->getDocumentElement();

            if (mixer->hasTagName("mixer")) {
                for (auto* child = mixer->getFirstChildElement(); child != nullptr; child = child->getNextElement()) {
                    if (child->hasTagName("channel") && child->getAttributeValue(2) == "active") {
                        auto index = atoi(child->getAttributeValue(0).getCharPointer());
                        outputs.insert(index, child->getStringAttribute("name"));
                    }
                    else {
                        // Handle inactive channels, maybe creating
                        // a inactiveOutputs[] array??
                    }
                };
            }
            else {
                jassertfalse; // Wrong tag name inside xml
            }
        }
        else {
            jassertfalse; // mixer_info.xml not found in /Source/utils/
        }
    }

    ~DrumsetXmlHandler()
    { }

    /*
        Get active output channel names as StringArray
    */
    StringArray DrumsetXmlHandler::getActiveOutputs() { return outputs; }

private:
    File file;
    StringArray outputs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumsetXmlHandler)
};
