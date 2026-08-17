// Copyright 2025 Isaac Hsu

#pragma once

#include "Expression.h"


namespace AlgebGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CFactAssignment // Used to store an assignment operation on a fact
    {
        friend class CEffect;
    public:
        CFactAssignment(const CBooleanFact& Fact, bool Right);
        CFactAssignment(const CBooleanFact& Fact, const CBooleanFact& Right);
        CFactAssignment(const CBooleanFact& Fact, const CBooleanExpression& Right);

        CFactAssignment(const CEnumerationFact& Fact, int Right);
        CFactAssignment(const CEnumerationFact& Fact, const CEnumerationFact& Right);

        CFactAssignment(const CNumericFact& Fact, EOperator Operator, const COperand& Right);
        CFactAssignment(const CNumericFact& Fact, EOperator Operator, const CExpression& Right);

    private:
        const CFact& mSubject;
        CExpression mExpression;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////  
}
