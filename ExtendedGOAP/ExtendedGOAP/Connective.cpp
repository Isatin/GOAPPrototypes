// Copyright 2026 Isaac Hsu

#include <cassert>

#include "Connective.h"
#include "Utility.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool EConnective::operator () (bool Left, bool Right) const
{
    switch (mValue)
    {
    case nil:           return Right;
    case logicalNOT:    return !Right;
    case logicalAND:    return Left && Right;
    case logicalOR:     return Left || Right;
    }

    assert(!"Invalid connective type");
    return false;
}

ETriState EConnective::operator () (ETriState Left, ETriState Right) const
{
    if (mValue == nil)
    {
        return Right;
    }
    else if (mValue == logicalNOT)
    {
        switch (Right)
        {
        case ETriState::no:     return ETriState::yes;
        case ETriState::yes:    return ETriState::no;
        }

        return ETriState::unknown;
    }
    else if (mValue == logicalAND)
    {
        if (Left == ETriState::no)
        {
            return ETriState::no;
        }
        else if (Right == ETriState::no)
        {
            return ETriState::no;
        }
        else if (Left == ETriState::yes && Right == ETriState::yes)
        {
            return ETriState::yes;
        }

        return ETriState::unknown;
    }
    else if (mValue == logicalOR)
    {
        if (Left == ETriState::yes)
        {
            return ETriState::yes;
        }
        else if (Right == ETriState::yes)
        {
            return ETriState::yes;
        }
        else if (Left == ETriState::no && Right == ETriState::no)
        {
            return ETriState::no;
        }

        return ETriState::unknown;
    }

    assert(!"Invalid connective type");
    return ETriState::unknown;
}

bool EConnective::IsCommutative() const
{
    switch (mValue)
    {
    case nil:           return false;
    case logicalNOT:    return false;
    case logicalAND:    return true;
    case logicalOR:     return true;
    }

    return false;
}

int EConnective::GetArity() const
{
    switch (mValue)
    {
    case nil:           return 1; // The nil connective means evaluating only the right operand.
    case logicalNOT:    return 1;
    case logicalAND:    return 2;
    case logicalOR:     return 2;
    }

    return 0;
}

const char* EConnective::GetName() const
{
    switch (mValue)
    {
    case nil:           return "Nil";
    case logicalNOT:    return "LogicalNOT";
    case logicalAND:    return "LogicalAND";
    case logicalOR:     return "LogicalOR";
    }

    return "UNDEF";
}

const char* EConnective::GetFunctionName() const
{
    switch (mValue)
    {
    case nil:           return "";
    case logicalNOT:    return "not";
    case logicalAND:    return "and";
    case logicalOR:     return "or";
    }

    return "UNDEF";
}

const char* EConnective::GetSymbol() const
{
    switch (mValue)
    {
    case nil:           return nullptr;
#ifdef USE_UNICODE_SYMBOLS
    case logicalNOT:    return "¬";
    case logicalAND:    return "∧";
    case logicalOR:     return "∨";
#else
    case logicalNOT:    return "!";
    case logicalAND:    return "&";
    case logicalOR:     return "|"; 
#endif
    }

    assert(!"Invalid connective type");
    return nullptr;
}

std::string EConnective::Stringize(const std::string& Left, const std::string& Right) const
{
    if (IsNil())
    {
        return Right;
    }
    else 
    {
        std::string Return;
        const int Arity = GetArity();
        const char* Symbol = GetSymbol();

        if (Arity == 1)
        {
            if (Symbol)
            {
                (Return = Symbol) += Right;
            }
            else if (IsParenthetic(Right))
            {
                (Return = GetFunctionName()) += Right;
            }
            else
            {
                (((Return = GetFunctionName()) += '(') += Right) += ')';
            }
        }
        else if (Arity == 2)
        {
            if (Symbol)
            {
                ((Return = Left) += Symbol) += Right;
            }
            else
            {
                (((((Return = GetFunctionName()) += '(') += Left) += ',') += Right) += ')';
            }
        }

        return Return;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
