#ifndef __TEXT_RECT_FAMILY_H__
#define __TEXT_RECT_FAMILY_H__

#include <vector>
#include <algorithm>
#include <allegro5/allegro5.h>
#include "TextRect.h"
#include "CropperDisplacer.h"
#include "Engine.h"

using std::vector;

class TextRectDropLocation {
public:
    virtual bool AcceptTextRect(TextRect* tr, const glm::ivec2& pos) = 0;
    virtual void RejectTextRect(TextRect* tr) = 0;
};

class TextRectDefaultDL : public TextRectDropLocation {
public:
    GuiElement& droplocation;

    TextRectDefaultDL(GuiElement& dl);
    bool AcceptTextRect(TextRect* tr, const glm::ivec2& pos);
    void RejectTextRect(TextRect* tr);
};

class TextRectCPDL : public TextRectDropLocation {
public:
    CropperDisplacer& droplocation;

    TextRectCPDL(CropperDisplacer& cpdl);
    bool AcceptTextRect(TextRect* tr, const glm::ivec2& pos);
    void RejectTextRect(TextRect* tr);
};

class TextRectFamily //: public GuiElement
{
private:
    vector<TextRect*> bigBrothers;

public:
    class Iterator {
    private:
        vector<TextRect*>::iterator bigBroIt, endBroIt;
        TextRect* curBro;

    public:
        Iterator(vector<TextRect*>& vec);
        Iterator(vector<TextRect*>& vec, int dummy);

        const bool operator !=(Iterator& r);
        void operator ++();
        TextRect* operator*();
    };

    Iterator begin();
    Iterator end();

    TextRectFamily();

    //void Draw();
    //Engine::InputStatus Event(ALLEGRO_EVENT& event);

    void promoteToBigBro(TextRect* tr);
    void demoteFromBigBro(TextRect* tr);

    vector<TextRectDropLocation*> dropLocations;
    void addDropLocation(TextRectDropLocation* droploc);

    TextRect* displacedBro = nullptr;

    bool shadowBro = false;
    glm::ivec2 shadowBroPos;
};

#endif //__TEXT_RECT_FAMILY_H__