// Copyright 2026 Isaac Hsu

#pragma once

#include <ostream>


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class ETriState // Mock enum of three-state Boolean
    {
    public:
        enum class Type
        {
            unknown = -1,
            no      = 0,
            yes     = 1
        };

        static constexpr Type unknown   = Type::unknown;
        static constexpr Type no        = Type::no;
        static constexpr Type yes       = Type::yes;

        // Because a class cannot contain static constexpr data members of its own incomplete type,
        // separate static instances of the enum and the class are declared for different purposes.
        static const ETriState Unknown;
        static const ETriState No;
        static const ETriState Yes;

    public:
        ETriState() = default;
        ETriState(Type Source) : mValue(Source) {}
        explicit ETriState(bool Source) : mValue(Source ? yes : no) {}

        ETriState& operator = (ETriState Source) { mValue = Source; return *this; }
        ETriState& operator = (bool Source) { mValue = Source ? yes : no; return *this; }
        operator Type() const { return mValue; }
        ETriState operator ! () const;

        Type Get() const { return mValue; }
        bool ToBool() const;
        const char* GetName() const;
        const char* GetSymbol() const;

        bool IsUnknown() const  { return mValue == unknown; }
        bool IsNo() const       { return mValue == no; }
        bool IsYes() const      { return mValue == yes; }

        friend std::ostream& operator << (std::ostream& Stream, ETriState Source) { return Stream << Source.GetName(); }

    private:
        Type mValue = ETriState::unknown;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}