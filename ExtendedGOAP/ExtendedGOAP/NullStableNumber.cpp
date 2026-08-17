// Copyright 2026 Isaac Hsu

#include "NullStableNumber.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    const CNullStableNumber CNullStableNumber::Null(CNumber::Null);

    CNullStableNumber& CNullStableNumber::operator += (CNumber Right)
    {
        if (IsNull())
        {
            if (!Right.IsNull())
            {
                mValue = Right;
            }
        }
        else
        {
            if (!Right.IsNull())
            {
                mValue += Right;
            }
        }

        return *this;
    }

    CNullStableNumber& CNullStableNumber::operator -= (CNumber Right)
    {
        if (IsNull())
        {
            if (!Right.IsNull())
            {
                mValue = -Right;
            }
        }
        else
        {
            if (!Right.IsNull())
            {
                mValue -= Right;
            }
        }

        return *this;
    }

    CNullStableNumber& CNullStableNumber::operator *= (CNumber Right)
    {
        if (!IsNull() && !Right.IsNull())
        {
            mValue *= Right;
        }

        return *this;
    }

    CNullStableNumber& CNullStableNumber::operator /= (CNumber Right)
    {
        if (!IsNull() && !Right.IsNull())
        {
            mValue /= Right;
        }

        return *this;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
}