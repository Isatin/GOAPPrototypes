// Copyright 2026 Isaac Hsu

#pragma once

#include "SimultaneousFormulas.h"


namespace ExtendedGOAP
{
    class CFactDefinition;
    class CState;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CCondition // Desired condition storing a logical formula or simultaneous formulas
    {
    public:
        CCondition(const CFactDefinition & Definition);
        virtual ~CCondition() {}

        virtual std::unique_ptr<CCondition> Clone() const { return std::make_unique<CCondition>(*this); }
        virtual std::string ToString() const { return mSimultaneousFormulas.ToString(mDefinition, false); }

        bool IsEmpty() const { return mSimultaneousFormulas.IsEmpty(); }
        CSimultaneousFormulas& GetConstraint()                  { return mSimultaneousFormulas; }
        const CSimultaneousFormulas& GetConstraint() const      { return mSimultaneousFormulas; }
        void SetConstraint(const CSimultaneousFormulas& Source) { mSimultaneousFormulas = Source; }
        void SetConstraint(CSimultaneousFormulas&& Source)      { mSimultaneousFormulas = std::move(Source); }
        void SetConstraint(const CFormula& Source)              { mSimultaneousFormulas = Source; }
        void SetConstraint(CFormula&& Source)                   { mSimultaneousFormulas = std::move(Source); }
        void ClearConstraint() { mSimultaneousFormulas.Clear(); }

        const CFactDefinition& GetDefinition() const { return mDefinition; }
        CBitVector GetUsedFactBits() const;
        bool IsSatisfiedBy(const CState& State) const;

        template <typename T>
        void ReplaceFacts(T&& Replacements) { mSimultaneousFormulas.ReplaceFacts(std::forward<T>(Replacements)); }
        void Rearrange();

    private:
        const CFactDefinition& mDefinition;
        CSimultaneousFormulas mSimultaneousFormulas;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
