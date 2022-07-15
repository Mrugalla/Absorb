#pragma once
#include "Knob.h"
#include "SplineEditor.h"

namespace gui
{
    struct LowLevel :
        public Comp
    {
        LowLevel(Utils& u) :
            Comp(u, "", CursorType::Default),
            rm(u),
            am(u),
            shapr(u),
            crushr(u),
            foldr(u)
        {
            layout.init(
                { 1, 3, 3, 3, 3, 3, 1 },
                { 1, 8, 1 }
            );

            makeParameter(rm, PID::AbsorbRM, "RM");
            addAndMakeVisible(rm);

            makeParameter(am, PID::AbsorbAM, "AM");
            addAndMakeVisible(am);

            makeParameter(shapr, PID::AbsorbShapr, "Shapr");
            addAndMakeVisible(shapr);

            makeParameter(crushr, PID::AbsorbCrushr, "Crushr");
            addAndMakeVisible(crushr);

            makeParameter(foldr, PID::AbsorbFoldr, "Foldr");
            addAndMakeVisible(foldr);
        }

    protected:
        Knob rm, am, shapr, crushr, foldr;
        
        void paint(Graphics& g) override
        {
            const auto thicc = utils.thicc;
            const auto thicc3 = thicc * 3.f;
            const Stroke stroke(thicc, Stroke::JointStyle::curved, Stroke::EndCapStyle::rounded);

            g.setColour(Colours::c(ColourID::Hover));

            const auto texturesBounds = layout(1.f, 1.f, 5.f, 1.f);
            drawRectEdges(g, texturesBounds, thicc3, stroke);
        }

        void resized() override
        {
            layout.resized();

            layout.place(rm, 1, 1, 1, 1, true);
            layout.place(am, 2, 1, 1, 1, true);
            layout.place(shapr, 3, 1, 1, 1, true);
            layout.place(crushr, 4, 1, 1, 1, true);
            layout.place(foldr, 5, 1, 1, 1, true);
        }
    };
}