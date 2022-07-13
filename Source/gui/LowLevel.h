#pragma once
#include "Knob.h"
#include "SplineEditor.h"

namespace gui
{
    struct LowLevel :
        public Comp
    {
        LowLevel(Utils& u) :
            Comp(u, "", CursorType::Default)
        {
        }

    protected:
        
        void paint(Graphics&) override {}

        void resized() override
        {
            layout.resized();
        }
    };
}