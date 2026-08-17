// Copyright 2025 Isaac Hsu

#pragma once

#include <vector>

#include "Expression.h"
#include "NullStableNumber.h"


namespace AlgebGOAP
{
    class CFactDefinition;
    class CNumericFact;
    class CStateBase;
    ///////////////////////////////////////////////////////////////////////////////////////////////    
    struct STerm // Term of algebraic expressions
    {
        CNumber Coefficient = 1;
        CExpression Expression;

    public:
        STerm(const COperand& Operand) : Expression(Operand) {}
        STerm(const CExpression& Expression) : Expression(Expression) {}
        STerm(const COperand& Operand, CNumber Coefficient) : Coefficient(Coefficient), Expression(Operand) {}
        STerm(const CExpression& Expression, CNumber Coefficient) : Coefficient(Coefficient), Expression(Expression) {}

        bool operator == (const STerm& Another) const;

        CNumber SplitInto(std::vector<STerm>& oTerms) const;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Collection of algebraic terms
    // TODO: Replace this class with one in a CAS library.
    class CTermList
    {
    public:
        static constexpr const char* GetEmptySymbol() { return "()"; }
        static const char* GetCoefficientTimesSymbol();

    public:
        CTermList() = default;
        explicit CTermList(CNumber Constant);
        explicit CTermList(const CExpression& Expression);

        CTermList operator + (const CTermList& Right) const;
        CTermList operator - (const CTermList& Right) const;
        CTermList operator * (CNumber Right) const;

        std::string ToString(const CFactDefinition& Definition) const { return ToString_Variadic(Definition); }
        std::string ToString() const { return ToString_Variadic(); }

        bool IsEmpty() const { return mConstantTerm.IsNull() && mTerms.empty(); }
        bool IsEqual(const CTermList& Another) const;
        // PREREQUISITE: Call Rearrange on both beforehand.
        bool IsEquivalent(const CTermList& Another) const;
        bool IsConstantTermEmpty() const { return mConstantTerm.IsNull() || mConstantTerm == 0; }
        int GetTermCount() const { return static_cast<int>(mTerms.size()); }
        // Return the first coefficient if available. Otherwise, return null.
        CNumber GetFirstCoefficient() const;
        // PREREQUISITE: Call Rearrange beforehand.
        CNumber GetConstantTerm() const { return mConstantTerm; }
        // Return 0 if the constant term is null or ±0. Otherwise, return it as is.
        CNumber GetUsableConstantTerm() const;
        CBitVector GetUsedFactBits() const;
        // PREREQUISITE: This list is not empty.
        const STerm& GetLastTerm() const { return mTerms.back(); }
        CNumber Evaluate(const CStateBase& State) const;
        [[nodiscard]] CTermList GetRearranged() const;
        [[nodiscard]] CExpression ToExpression() const;

        void SetConstantTerm(CNumber Value) { mConstantTerm = Value; }
        void ReplaceFact(const CNumericFact& Fact, const COperand& Replacement);
        void ReplaceFact(const CNumericFact& Fact, const CExpression& Replacement);
        void ReplaceFact(int FactIndex, const COperand& Replacement);
        void ReplaceFact(int FactIndex, const CExpression& Replacement);
        // Rearrange the operands into a consistent order so that equivalent terms yield the same form.
        void Rearrange();
        // Multiply these terms by -1 if the first coefficient is negative and return true. Otherwise, return false and do nothing. 
        bool PositivizeFirstCoefficient();
        // Normalize the first coefficient and proportionally scale the rest. Return the scale value.
        CNumber NormalizeFirstCoefficientAndScaleOthers();

    private:
        template <typename... TArgs>
        std::string ToString_Variadic(TArgs&&... Args) const;
        void CombineLikeTermsAndSortTerms();

    private:
        CNullStableNumber mConstantTerm = CNullStableNumber::Null;
        std::vector<STerm> mTerms;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename... TArgs>
    std::string CTermList::ToString_Variadic(TArgs&&... Args) const
    {
        std::string Return;

        if (mTerms.empty())
        {
            if (!mConstantTerm.IsNull())
            {
                Return += mConstantTerm.ToString();
            }
        }
        else
        {
            for (int i = 0; i < mTerms.size(); i++)
            {
                const STerm& Term = mTerms[i];

                if (Term.Coefficient == 1)
                {
                    if (i > 0)
                    {
                        Return += '+';
                    }
                }
                else if (Term.Coefficient == -1)
                {
                    Return += '-';
                }
                else
                {
                    if (Term.Coefficient >= 0 && i > 0)
                    {
                        Return += '+';
                    }

                    Return += Term.Coefficient.ToString();
                    Return += GetCoefficientTimesSymbol();
                }

                if (Term.Expression.IsOnlyOneOperand())
                {
                    Return += Term.Expression.ToString(std::forward<TArgs>(Args)...);
                }
                else
                {
                    Return += '(';
                    Return += Term.Expression.ToString(std::forward<TArgs>(Args)...);
                    Return += ')';
                }
            }

            if (!IsConstantTermEmpty())
            {
                Return += mConstantTerm.ToSignedString();
            }
        }

        return Return.empty() ? GetEmptySymbol() : Return;
    }
}

