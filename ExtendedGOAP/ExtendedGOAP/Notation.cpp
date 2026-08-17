// Copyright 2026 Isaac Hsu

#include "BooleanExpression.h"
#include "Fact.h"
#include "MatrixExpression.h"
#include "Notation.h"
#include "SetExpression.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
CFactAssignment::CFactAssignment(const CBooleanFact& Fact, bool Right)
    : mSubject(Fact)
    , mExpression(EOperator::nil, COperand::Null, Right)
{}

CFactAssignment::CFactAssignment(const CBooleanFact& Fact, const CBooleanFact& Right)
    : mSubject(Fact)
    , mExpression(EOperator::nil, COperand::Null, Right)
{}

CFactAssignment::CFactAssignment(const CBooleanFact& Fact, const CBooleanExpression& Right)
    : mSubject(Fact)
    , mExpression(Right.ToBase())
{}

CFactAssignment::CFactAssignment(const CEnumerationFact& Fact, int Right)
    : mSubject(Fact)
    , mExpression(EOperator::nil, COperand::Null, Right)
{}

CFactAssignment::CFactAssignment(const CEnumerationFact& Fact, const CEnumerationFact& Right)
    : mSubject(Fact)
    , mExpression(EOperator::nil, COperand::Null, Right)
{}

CFactAssignment::CFactAssignment(const CNumericFact& Fact, EOperator Operator, const COperand& Right)
    : mSubject(Fact)
    , mExpression(Operator, (Operator == EOperator::nil) ? COperand::Null : Fact, Right)
{}

CFactAssignment::CFactAssignment(const CNumericFact& Fact, EOperator Operator, const CExpression& Right)
    : mSubject(Fact)
    , mExpression(Right)
{
    // If this is a compound assignment, add an operation with the given operator.
    if (Operator != EOperator::nil)
    {
        mExpression.Prepend(Operator, mSubject);
    }
}

CFactAssignment::CFactAssignment(const CMatrixFact& Fact, EOperator Operator, const CNumericFact& Right)
    : mSubject(Fact)
    , mExpression(Operator, (Operator == EOperator::nil) ? COperand::Null : Fact, Right)
{}

CFactAssignment::CFactAssignment(const CMatrixFact& Fact, EOperator Operator, const CMatrixFact& Right)
    : mSubject(Fact)
    , mExpression(Operator, (Operator == EOperator::nil) ? COperand::Null : Fact, Right)
{}

CFactAssignment::CFactAssignment(const CMatrixFact& Fact, EOperator Operator, const CMatrixExpression& Right)
    : mSubject(Fact)
    , mExpression(Right.ToBase())
{
    // If this is a compound assignment, add an operation with the given operator.
    if (Operator != EOperator::nil)
    {
        mExpression.Prepend(Operator, mSubject);
    }
}

CFactAssignment::CFactAssignment(const CSetFact& Fact, EOperator Operator, const CSetFact& Right)
    : mSubject(Fact)
    , mExpression(Operator, (Operator == EOperator::nil) ? COperand::Null : Fact, Right)
{}

CFactAssignment::CFactAssignment(const CSetFact& Fact, EOperator Operator, const CSetExpression& Right)
    : mSubject(Fact)
    , mExpression(Right.ToBase())
{
    // If this is a compound assignment, add an operation with the given operator.
    if (Operator != EOperator::nil)
    {
        mExpression.Prepend(Operator, mSubject);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
