// Copyright 2024 Isaac Hsu

#include "Fact.h"
#include "Notation.h"


using namespace ArithGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////
SFactOperation ArithGOAP::operator ! (const CBooleanFact& Fact)
{ 
    return {Fact, EOperator::negation, 0}; 
}

SFactOperation ArithGOAP::operator - (const CNumericFact& Fact)
{ 
    return {Fact, EOperator::multiplication, -1}; 
}
///////////////////////////////////////////////////////////////////////////////////////////////
SFactEquation ArithGOAP::operator == (const CBooleanFact& Fact, bool Value)
{ 
    return SFactEquation(Fact, Value);
}

SFactEquation ArithGOAP::operator == (const CEnumerationFact& Fact, int Value)
{ 
    return SFactEquation(Fact, Value); 
}
///////////////////////////////////////////////////////////////////////////////////////////////////

