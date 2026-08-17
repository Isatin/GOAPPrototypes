// Copyright 2026 Isaac Hsu

#include <algorithm>
#include <cassert>

#include "BitVector.h"
#include "Debug.h"
#include "Fact.h"
#include "Formula.h"
#include "Range.h"
#include "State.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////
struct CFormula::SRepeatedLogicalOperation
{
    EConnective Connective;
    int SourceCompoundIndex = InvalidIndex; // Compound index of the source formula from which these repeated logical operations are tracked
    ETriState Result;                       // Combined Boolean constant from the repeated operations
    std::vector<int> RelationIndexes;       // Indexes of referenced relations 
    std::vector<int> CompoundIndexes;       // Indexes of referenced compounds

public:
    static bool IsSourceCompoundIndexGreaterThanAnother(const SRepeatedLogicalOperation& Left, const SRepeatedLogicalOperation& Right);

    CFormula ToFormula() const;
    void UpdateCompoundIndexes(const std::unordered_map<int, int>& CompoundIndexMap);
};

bool CFormula::SRepeatedLogicalOperation::IsSourceCompoundIndexGreaterThanAnother(const SRepeatedLogicalOperation& Left, const SRepeatedLogicalOperation& Right)
{
    return Left.SourceCompoundIndex > Right.SourceCompoundIndex;
}

CFormula CFormula::SRepeatedLogicalOperation::ToFormula() const
{
    assert(Connective.GetArity() > 0);
    assert(Connective.GetArity() != 1 || CompoundIndexes.size() + RelationIndexes.size() <= 1);

    CFormula Return;
    std::vector<SCompound>& Compounds = Return.mCompounds;
    int IndexOfCompoundIndexes = 0;
    int IndexOfRelationIndexes = 0;
    ETriState Constant = (Result == Connective.GetIdentity()) ? ETriState::Unknown : Result;

    if (CompoundIndexes.size() >= 2)
    {
        Compounds.emplace_back(Connective,
            CPredicate(EPredicate::compound, CompoundIndexes[IndexOfCompoundIndexes]),
            CPredicate(EPredicate::compound, CompoundIndexes[IndexOfCompoundIndexes + 1]));
        IndexOfCompoundIndexes += 2;
    }
    else if (CompoundIndexes.size() == 1)
    {
        if (RelationIndexes.size() >= 1)
        {
            Compounds.emplace_back(Connective,
                CPredicate(EPredicate::compound, CompoundIndexes[IndexOfCompoundIndexes++]),
                CPredicate(EPredicate::atomic, RelationIndexes[IndexOfRelationIndexes++]));
        }
        else if (!Constant.IsUnknown())
        {
            Compounds.emplace_back(Connective, CPredicate(EPredicate::compound, CompoundIndexes[IndexOfCompoundIndexes++]), CPredicate(Constant.ToBool()));
            Constant = ETriState::unknown;
        }
        else
        {
            Compounds.emplace_back(EPredicate::compound, CompoundIndexes[IndexOfCompoundIndexes++]);
        }
    }
    else // If no fact references exist
    {
        if (RelationIndexes.size() >= 2)
        {
            Compounds.emplace_back(Connective,
                CPredicate(EPredicate::atomic, RelationIndexes[IndexOfRelationIndexes]),
                CPredicate(EPredicate::atomic, RelationIndexes[IndexOfRelationIndexes + 1]));
            IndexOfRelationIndexes += 2;
        }
        else if (RelationIndexes.size() == 1)
        {
            if (!Constant.IsUnknown())
            {
                Compounds.emplace_back(Connective, CPredicate(EPredicate::atomic, RelationIndexes[IndexOfRelationIndexes++]), CPredicate(Constant.ToBool()));
                Constant = ETriState::unknown;
            }
            else
            {
                Compounds.emplace_back(EPredicate::atomic, RelationIndexes[IndexOfRelationIndexes++]);
            }
        }
        else if (!Result.IsUnknown()) // If only a constant is available
        {
            Compounds.emplace_back(Result.ToBool());
            Constant = ETriState::unknown;
        }
    }

    for (; IndexOfCompoundIndexes < CompoundIndexes.size(); IndexOfCompoundIndexes++)
    {
        Compounds.emplace_back(Connective,
            CPredicate(EPredicate::compound, SourceCompoundIndex + Compounds.size() - 1),
            CPredicate(EPredicate::compound, CompoundIndexes[IndexOfCompoundIndexes]));
    }

    for (; IndexOfRelationIndexes < RelationIndexes.size(); IndexOfRelationIndexes++)
    {
        Compounds.emplace_back(Connective,
            CPredicate(EPredicate::compound, SourceCompoundIndex + Compounds.size() - 1),
            CPredicate(EPredicate::atomic, RelationIndexes[IndexOfRelationIndexes]));
    }

    if (!Constant.IsUnknown() && Constant != Connective.GetIdentity())
    {
        Compounds.emplace_back(Connective,
            CPredicate(EPredicate::compound, SourceCompoundIndex + Compounds.size() - 1),
            CPredicate(Constant.ToBool()));
    }

    return Return;
}

void CFormula::SRepeatedLogicalOperation::UpdateCompoundIndexes(const std::unordered_map<int, int>& CompoundIndexMap)
{
    if (CompoundIndexMap.empty())
    {
        return;
    }

    for (int& CompoundIndex : CompoundIndexes)
    {
        auto it = CompoundIndexMap.find(CompoundIndex);
        if (it != CompoundIndexMap.end())
        {
            CompoundIndex = it->second;
        }
    }

    auto it = CompoundIndexMap.find(SourceCompoundIndex);
    if (it != CompoundIndexMap.end())
    {
        SourceCompoundIndex = it->second;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const CPredicate CPredicate::Null;

CPredicate::CPredicate(bool Value)
    : mType(EPredicate::boolean)
    , mBool(Value) 
{}

CPredicate::CPredicate(EPredicate Type, int Index) 
    : mType(Type)
    , mIndex(Index) 
{
    assert(Type != EPredicate::boolean);
}

CPredicate::CPredicate(EPredicate Type, size_t Index) 
    : CPredicate(Type, static_cast<int>(Index))
{}

bool CPredicate::operator == (const CPredicate& Another) const
{
    if (mType != Another.mType)
    {
        return false;
    }

    if (mIndex != Another.mIndex)
    {
        return false;
    }

    switch (mType)
    {
    case EPredicate::null:     return true;
    case EPredicate::boolean:  return mBool == Another.mBool;
    case EPredicate::atomic:   return mIndex == Another.mIndex;
    case EPredicate::compound: return mIndex == Another.mIndex;
    }

    return true;
}

bool CPredicate::GetBool() const
{
    assert(IsBool());

    return mBool; 
}

int CPredicate::GetRelationIndex() const
{
    assert(IsAtomic());

    return mIndex;
}

void CPredicate::SetRelationIndex(int RelationIndex)
{
    assert(IsAtomic());

    mIndex = RelationIndex;
}

int CPredicate::GetCompoundIndex() const
{
    assert(IsCompound());

    return mIndex;
}

void CPredicate::SetCompoundIndex(int CompoundIndex)
{
    assert(IsCompound());

    mIndex = CompoundIndex;
}

void CPredicate::SetNull()
{
    mType = EPredicate::null;
    mIndex = InvalidIndex;
}

void CPredicate::SetBool(bool Value)
{
    mType = EPredicate::boolean;
    mBool = Value;
}

void CPredicate::SetAtom(int RelationIndex)
{
    mType = EPredicate::atomic;
    mIndex = RelationIndex;
}

void CPredicate::SetCompound(int CompoundIndex)
{
    mType = EPredicate::compound;
    mIndex = CompoundIndex;
}

void CPredicate::ShiftIndexes(int CompoundOffset, int RelationOffset)
{
    if (IsAtomic())
    {
        mIndex += RelationOffset;
    }
    else if (IsCompound())
    {
        mIndex += CompoundOffset;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
SCompound::SCompound(bool Value)
    : Connective(EConnective::nil)
    , Right(CPredicate(Value))
{}

SCompound::SCompound(EPredicate Type, int Index)
    : Connective(EConnective::nil)
    , Right(CPredicate(Type, Index))
{}

SCompound::SCompound(EConnective Connective, const CPredicate& Predicate)
    : Connective(Connective)
    , Right(Predicate)
{}

SCompound::SCompound(EConnective Connective, const CPredicate& Left, const CPredicate& Right)
    : Connective(Connective)
    , Left(Left)
    , Right(Right)
{
    assert((Connective.GetArity() == 2 && !Left.IsNull() && !Right.IsNull()) || (Connective.GetArity() == 1 && Left.IsNull() && !Right.IsNull()));
}

bool SCompound::operator == (const SCompound& Another) const
{
    return Connective == Another.Connective
        && Left == Another.Left
        && Right == Another.Right;
}

bool SCompound::IsOnlyOneBool() const
{
    if (HasConnective())
    {
        return false;
    }

    switch (Right.GetType())
    {
    case EPredicate::null:     return false;
    case EPredicate::boolean:  return true;
    case EPredicate::atomic:   return false;
    case EPredicate::compound: return false;
    }

    assert(!"Invalid right-hand-side predicate type");
    return false;
}

bool SCompound::IsOnlyOneAtom() const
{
    if (HasConnective())
    {
        return false;
    }

    switch (Right.GetType())
    {
    case EPredicate::null:     return false;
    case EPredicate::boolean:  return false;
    case EPredicate::atomic:   return true;
    case EPredicate::compound: return false;
    }

    assert(!"Invalid right-hand-side predicate type");
    return false;
}

int SCompound::GrabTheOnlyRelationIndex() const
{
    assert(IsOnlyOneAtom());

    return Right.GetRelationIndex();
}

void SCompound::SetTo(bool Value)
{
    Connective = EConnective::nil;
    Left.SetNull();
    Right.SetBool(Value);
}

void SCompound::SetTo(const CPredicate& Predicate)
{
    assert(!Predicate.IsNull());

    Connective = EConnective::nil;
    Right = Predicate; // Set the right before the left in case the parameter is the left itself.
    Left.SetNull();
}

void SCompound::ShiftIndexes(int CompoundOffset, int RelationOffset)
{
    Left.ShiftIndexes(CompoundOffset, RelationOffset);
    Right.ShiftIndexes(CompoundOffset, RelationOffset);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CFormula::CFormula(bool Value)
{
    mCompounds.emplace_back(Value);
}

CFormula::CFormula(const CRelation& Relation)
{
    assert(!Relation.IsEmpty());

    mRelations.push_back(Relation);
    mCompounds.emplace_back(EPredicate::atomic, 0);
}

CFormula::CFormula(CRelation&& Relation)
{
    assert(!Relation.IsEmpty());

    mRelations.emplace_back(std::move(Relation));
    mCompounds.emplace_back(EPredicate::atomic, 0);
}

CFormula::CFormula(EConnective Connective, const CRelation& Relation)
{
    assert(Connective.GetArity() == 1);
    assert(!Relation.IsEmpty());

    mRelations.push_back(Relation);
    mCompounds.emplace_back(Connective, CPredicate(EPredicate::atomic, 0));
}

CFormula::CFormula(EConnective Connective, CRelation&& Relation)
{
    assert(Connective.GetArity() == 1);
    assert(!Relation.IsEmpty());

    mRelations.emplace_back(std::move(Relation));
    mCompounds.emplace_back(Connective, CPredicate(EPredicate::atomic, 0));
}

CFormula::CFormula(EConnective Connective, const CFormula& Formula)
    : CFormula(Formula)
{
    assert(Connective.GetArity() == 1);
    assert(!Formula.IsEmpty());

    if (IsOnlyOnePredicate())
    {
        mCompounds.front().Connective = Connective;
    }
    else
    {
        mCompounds.emplace_back(Connective, CPredicate(EPredicate::compound, mCompounds.size() - 1));
    }
}

CFormula::CFormula(EConnective Connective, CFormula&& Formula)
    : CFormula(std::move(Formula))
{
    assert(Connective.GetArity() == 1);
    assert(!IsEmpty());

    if (IsOnlyOnePredicate())
    {
        mCompounds.front().Connective = Connective;
    }
    else
    {
        mCompounds.emplace_back(Connective, CPredicate(EPredicate::compound, mCompounds.size() - 1));
    }
}

CFormula::CFormula(EConnective Connective, const CRelation& Left, const CRelation& Right)
{
    assert(Connective.GetArity() == 2);
    assert(!Left.IsEmpty());
    assert(!Right.IsEmpty());

    mRelations.push_back(Left);
    mRelations.push_back(Right);
    mCompounds.emplace_back(Connective, CPredicate(EPredicate::atomic, 0), CPredicate(EPredicate::atomic, 1));
}

CFormula::CFormula(EConnective Connective, CRelation&& Left, CRelation&& Right)
{
    assert(Connective.GetArity() == 2);
    assert(!Left.IsEmpty());
    assert(!Right.IsEmpty());

    mRelations.emplace_back(std::move(Left));
    mRelations.emplace_back(std::move(Right));
    mCompounds.emplace_back(Connective, CPredicate(EPredicate::atomic, 0), CPredicate(EPredicate::atomic, 1));
}

CFormula::CFormula(EConnective Connective, const CRelation& Left, const CFormula& Right)
    : CFormula(Right)
{
    assert(Connective.GetArity() == 2);
    assert(!Left.IsEmpty());
    assert(!Right.IsEmpty());

    mRelations.push_back(Left);
    mCompounds.emplace_back(Connective, CPredicate(EPredicate::atomic, mRelations.size() - 1), CPredicate(EPredicate::compound, mCompounds.size() - 1));
}

CFormula::CFormula(EConnective Connective, CRelation&& Left, CFormula&& Right)
    : CFormula(std::move(Right))
{
    assert(Connective.GetArity() == 2);
    assert(!Left.IsEmpty());
    assert(!IsEmpty());

    mRelations.emplace_back(std::move(Left));
    mCompounds.emplace_back(Connective, CPredicate(EPredicate::atomic, mRelations.size() - 1), CPredicate(EPredicate::compound, mCompounds.size() - 1));
}

CFormula::CFormula(EConnective Connective, const CFormula& Left, const CRelation& Right)
    : CFormula(Left)
{
    assert(Connective.GetArity() == 2);
    assert(!Left.IsEmpty());
    assert(!Right.IsEmpty());

    mRelations.push_back(Right);
    mCompounds.emplace_back(Connective, CPredicate(EPredicate::compound, mCompounds.size() - 1), CPredicate(EPredicate::atomic, mRelations.size() - 1));
}

CFormula::CFormula(EConnective Connective, CFormula&& Left, CRelation&& Right)
    : CFormula(std::move(Left))
{
    assert(Connective.GetArity() == 2);
    assert(!IsEmpty());
    assert(!Right.IsEmpty());

    mRelations.emplace_back(std::move(Right));
    mCompounds.emplace_back(Connective, CPredicate(EPredicate::compound, mCompounds.size() - 1), CPredicate(EPredicate::atomic, mRelations.size() - 1));
}

CFormula::CFormula(EConnective Connective, const CFormula& Left, const CFormula& Right)
    : CFormula(Left)
{
    assert(Connective.GetArity() == 2);
    assert(!Left.IsEmpty());
    assert(!Right.IsEmpty());

    Append(Connective, Right);
}

CFormula::CFormula(EConnective Connective, CFormula&& Left, CFormula&& Right)
    : CFormula(std::move(Left))
{
    assert(Connective.GetArity() == 2);
    assert(!IsEmpty());
    assert(!Right.IsEmpty());

    Append(Connective, std::move(Right));
}

bool CFormula::BuildFormula(CFormula& oFormula, EConnective Connective, const CExpression& Expression, const CRange& Range)
{
    assert(Connective.IsAND() || Connective.IsOR());
    assert(Range.IsSorted());

    if (Range.IsEmpty())
    {
        oFormula.SetTo(false);
        return true;
    }
    if (Range.IsFull())
    {
        oFormula.SetTo(true);
        return true;
    }
    if (Expression.IsEmpty())
    {
        oFormula.Clear();
        return true;
    }

    if (Range.GetSpanCount() == 1)
    {
        oFormula.AddRelations(Expression, Range.GrabFirstSpan());
    }
    else if (Connective.IsAND())
    {
        const SSpan& FirstSpan = Range.GrabFirstSpan();
        const SSpan& LastSpan = Range.GrabLastSpan();
        CNumber PrevSupremum = FirstSpan.Supremum;
        ESpanType PrevSupremumType = FirstSpan.SupremumType;
        if (!PrevSupremumType.IsApproximationExclusive())
        {
            return false;
        }

        for (int i = 1; i < Range.GetSpanCount(); i++)
        {
            const SSpan& Span = Range.GetSpan(i);
            if (!PrevSupremum.IsEqual(Span.Infimum))
            {
                return false;
            }
            if (!Span.InfimumType.IsApproximationExclusive())
            {
                return false;
            }

            oFormula.AddRelation(EComparer::NotAlmostEqual, Expression, PrevSupremum);
            PrevSupremum = Span.Supremum;
            PrevSupremumType = Span.SupremumType;
        }

        oFormula.AddRelation(FirstSpan.InfimumType.GetInfimumComparer(), Expression, FirstSpan.Infimum);
        oFormula.AddRelation(LastSpan.SupremumType.GetSuprenumComparer(), Expression, LastSpan.Supremum);
    }
    else if (Connective.IsOR())
    {
        const SSpan& FirstSpan = Range.GrabFirstSpan();
        const SSpan& LastSpan = Range.GrabLastSpan();
        CNumber PrevSupremum = FirstSpan.Supremum;
        ESpanType PrevSupremumType = FirstSpan.SupremumType;

        for (int i = 1; i < Range.GetSpanCount(); i++)
        {
            const SSpan& Span = Range.GetSpan(i);
            if (PrevSupremum.IsEqual(Span.Infimum) && PrevSupremumType.IsApproximationExclusive() && Span.InfimumType.IsApproximationExclusive())
            {
                oFormula.AddRelation(EComparer::NotAlmostEqual, Expression, PrevSupremum);
            }
            else
            {
                oFormula.AddRelation(PrevSupremumType.GetSuprenumComparer(), Expression, PrevSupremum);
                oFormula.AddRelation(Span.InfimumType.GetInfimumComparer(), Expression, Span.Infimum);
            }

            PrevSupremum = Span.Supremum;
            PrevSupremumType = Span.SupremumType;
        }

        oFormula.AddRelation(FirstSpan.InfimumType.GetInfimumComparer(), Expression, FirstSpan.Infimum);
        oFormula.AddRelation(LastSpan.SupremumType.GetSuprenumComparer(), Expression, LastSpan.Supremum);
    }

    if (oFormula.mRelations.size() == 1)
    {
        oFormula.mCompounds.emplace_back(EPredicate::atomic, 0);
    }
    else if (oFormula.mRelations.size() > 1)
    {
        oFormula.mCompounds.emplace_back(Connective, CPredicate(EPredicate::atomic, 0), CPredicate(EPredicate::atomic, 1));

        for (int RelationIndex = 2; RelationIndex < oFormula.mRelations.size(); RelationIndex++)
        {
            oFormula.mCompounds.emplace_back(Connective,
                CPredicate(EPredicate::compound, oFormula.mCompounds.size() - 1),
                CPredicate(EPredicate::atomic, RelationIndex));
        }
    }

    return true;
}

void CFormula::AddRelations(const CExpression& Expression, const SSpan& Span)
{
    if (Span.Infimum.IsEqual(Span.Supremum) && Span.InfimumType.IsApproximate() && Span.SupremumType.IsApproximate())
    {
        AddRelation(EComparer::AlmostEqual, Expression, Span.Infimum);
    }
    else
    {
        AddRelation(Span.InfimumType.GetInfimumComparer(), Expression, Span.Infimum);
        AddRelation(Span.SupremumType.GetSuprenumComparer(), Expression, Span.Supremum);
    }
}

bool CFormula::AddRelation(EComparer Comparer, const CExpression& Left, CNumber Right)
{
    assert(!Left.IsEmpty());
    assert(!Right.IsNull());

    if (Comparer.IsTautological(Right))
    {
        return false;
    }

    mRelations.emplace_back(Comparer, Left, Right);
    return true;
}

std::string CFormula::ToString(int StartCompoundIndex) const
{
    std::vector<int> CompoundIndexes = GetUsedCompoundIndexes(StartCompoundIndex);
    std::ranges::sort(CompoundIndexes);
    return ToString(CompoundIndexes);
}

std::string CFormula::ToString(const std::vector<int>& CompoundIndexes) const
{
    assert(std::ranges::is_sorted(CompoundIndexes));

    if (CompoundIndexes.empty())
    {
        return GetEmptySymbol();
    }

    if (CompoundIndexes.size() == 1)
    {
        const SCompound& Compound = mCompounds[CompoundIndexes.front()];
        if (Compound.IsOnlyOneAtom())
        {
            int RelationIndex = Compound.GrabTheOnlyRelationIndex();
            return mRelations[RelationIndex].ToString();
        }
    }

    std::vector<std::string> Texts(mCompounds.size());

    for (int i = 0; i < static_cast<int>(CompoundIndexes.size()); i++)
    {
        int CompoundIndex = CompoundIndexes[i];
        const SCompound& Compound = mCompounds[CompoundIndex];
        std::string LeftString = StringizePredicate(Compound.Left, Texts);
        std::string RightString = StringizePredicate(Compound.Right, Texts);
        Texts[CompoundIndex] = Compound.Connective.Stringize(LeftString, RightString);

        if (!Compound.IsOnlyOneBool() && i < static_cast<int>(mCompounds.size()) - 1) // Parenthesize inner compounds except when there is only one bool.
        {
            Parenthesize(Texts[CompoundIndex]);
        }
    }

    return Texts[CompoundIndexes.back()];
}

bool CFormula::IsEqual(const CFormula& Another) const
{
    if (mCompounds != Another.mCompounds)
    {
        return false;
    }

    if (mRelations != Another.mRelations)
    {
        return false;
    }

    return true;
}

bool CFormula::IsEquivalent(const CFormula& Another) const
{
    return ToString() == Another.ToString();
}

bool CFormula::ValidateCompound(const SCompound& Compound) const
{
    return ValidateCompound(Compound, static_cast<int>(mCompounds.size()));
}

bool CFormula::ValidateCompound(const SCompound& Compound, int CompoundLimit) const
{
    auto ValidatePredicate = [&](const CPredicate& Predicate)
    {
        if (Predicate.IsAtomic())
        {
            return Predicate.GetRelationIndex() >= 0 && Predicate.GetRelationIndex() < mRelations.size();
        }
        else if (Predicate.IsCompound())
        {
            return Predicate.GetCompoundIndex() >= 0 && Predicate.GetCompoundIndex() < CompoundLimit;
        }
        else if (Predicate.IsBool())
        {
            return true;
        }

        return false;
    };

    if (Compound.Connective.GetArity() > 1 && !ValidatePredicate(Compound.Left))
    {
        return false;
    }

    if (!ValidatePredicate(Compound.Right))
    {
        return false;
    }

    return true;
}

std::vector<int> CFormula::GetInvalidCompoundIndexes() const
{
    std::vector<int> InvalidCompoundIndexes;
    InvalidCompoundIndexes.reserve(mCompounds.size());

    for (int i = 0; i < mCompounds.size(); i++)
    {
        if (!ValidateCompound(mCompounds[i], i))
        {
            InvalidCompoundIndexes.push_back(i);
        }
    }

    return InvalidCompoundIndexes;
}

bool CFormula::IsOnlyOneRelation() const
{
    return mCompounds.size() == 1 && mCompounds.front().IsOnlyOneAtom();
}

const CRelation& CFormula::GrabTheOnlyRelation() const
{
    assert(IsOnlyOneRelation());

    int RelationIndex = mCompounds.front().Right.GetRelationIndex();
    return mRelations[RelationIndex];
}

bool CFormula::IsOnlyOnePredicate() const
{
    return mCompounds.size() == 1 && !mCompounds.front().HasConnective();
}

const CPredicate& CFormula::GrabTheOnlyPredicate() const
{
    assert(IsOnlyOnePredicate());
    // Technically, the right-hand-side predicate could be null or a compound reference, but it shouldn't be.
    assert(mCompounds.front().Right.IsBool() || mCompounds.front().Right.IsAtomic());

    return mCompounds.front().Right;
}

bool CFormula::IsOnlyOneBool() const
{
    return mCompounds.size() == 1 && !mCompounds.front().HasConnective() && mCompounds.front().Right.IsBool();
}

bool CFormula::IsBool(bool Value) const
{
    if (!IsOnlyOneBool())
    {
        return false;
    }

    return GrabTheOnlyBool() == Value;
}

bool CFormula::GrabTheOnlyBool() const
{
    assert(IsOnlyOneBool());

    return mCompounds.front().Right.GetBool();
}

ETriState CFormula::TryGetTheOnlyBool() const
{
    if (IsOnlyOneBool())
    {
        return ETriState(GrabTheOnlyBool());
    }
    else
    {
        return ETriState::unknown;
    }
}

EConnective CFormula::GetCommonConnective() const
{
    if (IsEmpty())
    {
        return EConnective::invalid;
    }

    EConnective FirstConnective = mCompounds.front().Connective;
    for (int i = 1; i < mCompounds.size(); i++)
    {
        if (mCompounds[i].Connective != FirstConnective)
        {
            return EConnective::invalid;
        }
    }

    return FirstConnective;
}

bool CFormula::IsFactUsed(int FactIndex) const
{
    for (const CRelation& Relation : mRelations)
    {
        if (Relation.GetLeft().IsFactUsed(FactIndex))
        {
            return true;
        }

        if (Relation.GetRight().IsFactUsed(FactIndex))
        {
            return true;
        }
    }

    return false;
}

CBitVector CFormula::GetUsedFactBits() const
{
    CBitVector Return;

    for (const CRelation& Relation : mRelations)
    {
        Return |= Relation.GetLeft().GetUsedFactBits();
        Return |= Relation.GetRight().GetUsedFactBits();
    }

    return Return;
}

int CFormula::AddUpArity(const std::vector<int>& CompoundIndexes) const
{
    int Count = 0;

    for (int i = 0; i < CompoundIndexes.size(); i++)
    {
        int CompoundIndex = CompoundIndexes[i];
        const SCompound& Compound = mCompounds[CompoundIndex];
        Count += Compound.Connective.GetArity();
    }

    return Count;
}

CFormula CFormula::GetSubFormula(int StartCompoundIndex) const
{
    if (StartCompoundIndex < 0 || StartCompoundIndex >= mCompounds.size())
    {
        return {};
    }

    std::vector<int> UsedRelationIndexes;
    std::vector<int> UsedCompoundIndexes;
    GetUsedCompoundAndRelationIndexes(UsedCompoundIndexes, UsedRelationIndexes, StartCompoundIndex);

    // Add used relations to the return formula and build index mappings.
    CFormula Return;
    std::vector<int> NewRelationIndexes(mRelations.size(), InvalidIndex);
    for (int i = 0; i < UsedRelationIndexes.size(); i++)
    {
        int UsedRelationIndex = UsedRelationIndexes[i];
        NewRelationIndexes[UsedRelationIndex] = static_cast<int>(Return.mRelations.size());
        Return.mRelations.push_back(mRelations[UsedRelationIndex]);
    }

    std::vector<int> NewCompoundIndexes(StartCompoundIndex + 1, InvalidIndex);
    auto UpdateIndexes = [&](CPredicate& Predicate)
    {
        if (Predicate.IsAtomic())
        {
            int NewIndex = NewRelationIndexes[Predicate.GetRelationIndex()];
            assert(NewIndex >= 0);
            Predicate.SetRelationIndex(NewIndex);
        }
        else if (Predicate.IsCompound())
        {
            int NewIndex = NewCompoundIndexes[Predicate.GetCompoundIndex()];
            assert(NewIndex >= 0);
            Predicate.SetCompoundIndex(NewIndex);
        }
    };

    // Copy the referenced compounds and update their indexes.
    std::ranges::sort(UsedCompoundIndexes);
    for (int i = 0; i < UsedCompoundIndexes.size(); i++)
    {
        int CompoundIndex = UsedCompoundIndexes[i];
        NewCompoundIndexes[CompoundIndex] = static_cast<int>(Return.mCompounds.size());
        SCompound& NewCompound = Return.mCompounds.emplace_back(mCompounds[CompoundIndex]);
        UpdateIndexes(NewCompound.Left);
        UpdateIndexes(NewCompound.Right);
    }

    return Return;
}

ETriState CFormula::Evaluate(const CStateBase& State) const
{
    if (mCompounds.empty())
    {
        return ETriState::unknown;
    }

    // Evaluate these relations from first to last.
    std::vector<ETriState> RelationResults(mRelations.size(), ETriState::unknown);
    for (int i = 0; i < mRelations.size(); i++)
    {
        RelationResults[i] = mRelations[i].Evaluate(State);
    }

    std::vector<ETriState> CompoundResults(mCompounds.size(), ETriState::unknown);
    auto EvaluatePredicate = [&](const CPredicate& Predicate)
    {
        ETriState Result = ETriState::unknown;

        if (Predicate.IsBool())
        {
            Result = Predicate.GetBool();
        }
        else if (Predicate.IsAtomic())
        {
            Result = RelationResults[Predicate.GetRelationIndex()];
        }
        else if (Predicate.IsCompound())
        {
            Result = CompoundResults[Predicate.GetCompoundIndex()];
        }

        return Result;
    };

    // Evaluate these compound formulas from first to last.
    for (int i = 0; i < mCompounds.size(); i++)
    {
        const SCompound& Compound = mCompounds[i];
        ETriState LeftValue = EvaluatePredicate(Compound.Left);
        ETriState RightValue = EvaluatePredicate(Compound.Right);
        CompoundResults[i] = Compound.Connective(LeftValue, RightValue);
    }

    return CompoundResults.back();
}

CNumber CFormula::GetHeuristicCost(const CState& State, SDebugInfo* DebugInfo) const
{
    if (mCompounds.empty())
    {
        return 0;
    }

    const CFactDefinition& Def = State.GetDefinition();
    const CNumber BaseCost = Def.GetBaseRelationCost();
    std::vector<CNumber> RelationCosts(mRelations.size(), BaseCost);
    for (int i = 0; i < mRelations.size(); i++)
    {
        const CRelation& Relation = mRelations[i];
        const CNumber Gap = Relation.GetGap(State);
        std::pair<CNumber, CNumber> ValueAndWeight = Def.GetHeuristicCost(Gap, Relation.GetUsedFactBits());
        RelationCosts[i] = ValueAndWeight.first;

        if (DebugInfo)
        {
            DebugInfo->UpdateHeuristicValue(ValueAndWeight, *this, State);
        }
    }

    std::vector<CNumber> CompoundCosts(mCompounds.size(), BaseCost);
    auto FectchCost = [&](const CPredicate& Predicate)
    {
        switch (Predicate.GetType())
        {
        case EPredicate::null:     return 0_n;
        case EPredicate::boolean:  return 0_n;
        case EPredicate::atomic:   return RelationCosts[Predicate.GetRelationIndex()];
        case EPredicate::compound: return CompoundCosts[Predicate.GetCompoundIndex()];
        }

        assert(!"Invalid predicate type");
        return BaseCost;
    };

    for (int i = 0; i < mCompounds.size(); i++)
    {
        const SCompound& Compound = mCompounds[i];
        CNumber LeftCost = FectchCost(Compound.Left);
        CNumber RightCost = FectchCost(Compound.Right);

        switch (Compound.Connective)
        {
        case EConnective::nil:
            CompoundCosts[i] = RightCost;
            break; 
        case EConnective::logicalAND:
            CompoundCosts[i] = LeftCost + RightCost;
            break;
        case EConnective::logicalOR:
            CompoundCosts[i] = LeftCost < RightCost ? LeftCost : RightCost;
            break;
        case EConnective::logicalNOT:
            assert(!"Do not support logical NOT. Call ApplyDeMorgansLaws() beforehand.");
            return BaseCost;
        default:
            assert(!"Invalid connective type");
            return BaseCost;
        }
    }

    return CompoundCosts.back();
}

std::pair<int, int> CFormula::CountMostUsedCompound() const
{
    if (mCompounds.empty())
    {
        return {InvalidIndex, 0};
    }

    int MostRefIndex = InvalidIndex;
    int MaxRefCount = 0;
    std::vector<int> RefCounts(mCompounds.size(), 0);

    auto CountReference = [&](const CPredicate& Predicate)
    {
        if (Predicate.IsCompound())
        {
            int RefIndex = Predicate.GetCompoundIndex();
            RefCounts[RefIndex]++;

            if (MostRefIndex == InvalidIndex || MaxRefCount < RefCounts[RefIndex])
            {
                MostRefIndex = RefIndex;
                MaxRefCount = RefCounts[RefIndex];
            }
        }
    };

    std::vector<int> CompoundIndexes;
    CompoundIndexes.reserve(mCompounds.size());
    CompoundIndexes.push_back(static_cast<int>(mCompounds.size()) - 1);
    for (int i = 0; i < CompoundIndexes.size(); i++)
    {
        const int CompoundIndex = CompoundIndexes[i];
        const SCompound& Compound = mCompounds[CompoundIndex];
        CountReference(Compound.Left);
        CountReference(Compound.Right);
    }

    return {MostRefIndex, MaxRefCount};
}

std::pair<int, int> CFormula::CountMostUsedRelation() const
{
    if (mCompounds.empty())
    {
        return {InvalidIndex, 0};
    }

    int MostRefIndex = InvalidIndex;
    int MaxRefCount = 0;
    std::vector<int> RefCounts(mRelations.size(), 0);

    auto CountReference = [&](const CPredicate& Predicate)
    {
        if (Predicate.IsAtomic())
        {
            int RefIndex = Predicate.GetRelationIndex();
            RefCounts[RefIndex]++;

            if (MostRefIndex == InvalidIndex || MaxRefCount < RefCounts[RefIndex])
            {
                MostRefIndex = RefIndex;
                MaxRefCount = RefCounts[RefIndex];
            }
        }
    };

    std::vector<int> UsedCompoundIndexes = GetUsedCompoundIndexes();
    for (int i = 0; i < UsedCompoundIndexes.size(); i++)
    {
        int CompoundIndex = UsedCompoundIndexes[i];
        const SCompound& Compound = mCompounds[CompoundIndex];
        CountReference(Compound.Left);
        CountReference(Compound.Right);
    }

    return {MostRefIndex, MaxRefCount};
}

std::pair<int, int> CFormula::CountUnusedCompoundsAndRelations() const
{
    std::vector<int> CompoundIndexes;
    std::vector<int> RelationIndexes;
    GetUsedCompoundAndRelationIndexes(CompoundIndexes, RelationIndexes);

    int UnusedCompoundCount = static_cast<int>(mCompounds.size()) - static_cast<int>(CompoundIndexes.size());
    assert(UnusedCompoundCount >= 0);

    int UnusedRelationCount = static_cast<int>(mRelations.size()) - static_cast<int>(RelationIndexes.size());
    assert(UnusedRelationCount >= 0);
    return {UnusedCompoundCount, UnusedRelationCount};
}

void CFormula::GetUsedCompoundAndRelationIndexes(std::vector<int>& oCompoundIndexes, std::vector<int>& oRelationIndexes) const
{
    if (!mCompounds.empty())
    {
        GetUsedCompoundAndRelationIndexes(oCompoundIndexes, oRelationIndexes, static_cast<int>(mCompounds.size()) - 1);
    }
}

void CFormula::GetUsedCompoundAndRelationIndexes(std::vector<int>& oCompoundIndexes, std::vector<int>& oRelationIndexes, int StartCompoundIndex) const
{
    oCompoundIndexes = GetUsedCompoundIndexes(StartCompoundIndex);
    oRelationIndexes.reserve(StartCompoundIndex);
    for (int i = 0; i < oCompoundIndexes.size(); i++)
    {
        const int CompoundIndex = oCompoundIndexes[i];
        const SCompound& Compound = mCompounds[CompoundIndex];
        if (Compound.Left.IsAtomic())
        {
            assert(std::ranges::find(oRelationIndexes, Compound.Left.GetRelationIndex()) == oRelationIndexes.end());
            oRelationIndexes.push_back(Compound.Left.GetRelationIndex());
        }
        if (Compound.Right.IsAtomic())
        {
            assert(std::ranges::find(oRelationIndexes, Compound.Right.GetRelationIndex()) == oRelationIndexes.end());
            oRelationIndexes.push_back(Compound.Right.GetRelationIndex());
        }
    }
}

int CFormula::CountUnusedCompounds() const
{
    std::vector<int> CompoundIndexes = GetUsedCompoundIndexes();
    int UnusedCompoundCount = static_cast<int>(mCompounds.size()) - static_cast<int>(CompoundIndexes.size());
    assert(UnusedCompoundCount >= 0);
    return UnusedCompoundCount;
}

std::vector<int> CFormula::GetUsedCompoundIndexes() const
{
    return GetUsedCompoundIndexes(static_cast<int>(mCompounds.size()) - 1);
}

std::vector<int> CFormula::GetUsedCompoundIndexes(int StartCompoundIndex) const
{
    assert(StartCompoundIndex >= 0 && StartCompoundIndex < mCompounds.size());

    // Record the compounds directly or indirectly referenced by the given compound.
    std::vector<int> CompoundIndexes;
    CompoundIndexes.reserve(StartCompoundIndex + 1);
    CompoundIndexes.push_back(StartCompoundIndex);
    for (int i = 0; i < CompoundIndexes.size(); i++)
    {
        const int CompoundIndex = CompoundIndexes[i];
        const SCompound& Compound = mCompounds[CompoundIndex];
        if (Compound.Left.IsCompound())
        {
            assert(std::ranges::find(CompoundIndexes, Compound.Left.GetCompoundIndex()) == CompoundIndexes.end());
            CompoundIndexes.push_back(Compound.Left.GetCompoundIndex());
        }
        if (Compound.Right.IsCompound())
        {
            assert(std::ranges::find(CompoundIndexes, Compound.Right.GetCompoundIndex()) == CompoundIndexes.end());
            CompoundIndexes.push_back(Compound.Right.GetCompoundIndex());
        }
    }

    return CompoundIndexes;
}

int CFormula::CountUsedAtomicCompounds() const
{
    if (mCompounds.empty())
    {
        return 0;
    }

    int Count = 0;

    std::vector<int> CompoundIndexes;
    CompoundIndexes.reserve(mCompounds.size());
    CompoundIndexes.push_back(static_cast<int>(mCompounds.size()) - 1);

    auto CountAtomicCompound = [&](const CPredicate& Predicate)
    {
        if (!Predicate.IsCompound())
        {
            return;
        }

        int RefCompoundIndex = Predicate.GetCompoundIndex();
        assert(std::ranges::find(CompoundIndexes, RefCompoundIndex) == CompoundIndexes.end());
        CompoundIndexes.push_back(RefCompoundIndex);

        if (mCompounds[RefCompoundIndex].HasConnective())
        {
            return;
        }

        Count++;
    };

    for (int i = 0; i < CompoundIndexes.size(); i++)
    {
        const int CompoundIndex = CompoundIndexes[i];
        const SCompound& Compound = mCompounds[CompoundIndex];
        CountAtomicCompound(Compound.Left);
        CountAtomicCompound(Compound.Right);
    }

    return Count;
}

std::vector<int> CFormula::GetInvalidCompoundReferences() const
{
    std::vector<int> InvalidIndexes;
    InvalidIndexes.reserve(mCompounds.size());

    auto AddInvalidCompound = [&](const CPredicate& Predicate, int UpperBound)
    {
        if (Predicate.IsCompound())
        {
            int RefIndex = Predicate.GetCompoundIndex();
            if (RefIndex < 0 || RefIndex >= UpperBound)
            {
                if (std::ranges::find(InvalidIndexes, RefIndex) == InvalidIndexes.end())
                {
                    InvalidIndexes.push_back(RefIndex);
                }
            }
        }
    };

    for (int i = 0; i < mCompounds.size(); i++)
    {
        const SCompound& Compound = mCompounds[i];
        AddInvalidCompound(Compound.Left, i);
        AddInvalidCompound(Compound.Right, i);
    }

    return InvalidIndexes;
}

void CFormula::Clear()
{
    mCompounds.clear();
    mRelations.clear();
}

void CFormula::SetTo(bool Value)
{
    mCompounds.clear();
    mCompounds.emplace_back(Value);
    mRelations.clear();
}

void CFormula::Append(EConnective Connective, const CFormula& Right)
{
    Append(Connective, CFormula(Right));
}

void CFormula::Append(EConnective Connective, CFormula&& Right)
{
    assert(Connective.GetArity() == 2);

    if (Right.IsEmpty())
    {
        return;
    }

    if (IsEmpty())
    {
        *this = std::move(Right);
    }
    else if (IsOnlyOnePredicate())
    {
        CPredicate LeftPredicate = GrabTheOnlyPredicate();
        mCompounds = std::move(Right.mCompounds);

        const int LeftRelationSize = static_cast<int>(mRelations.size());
        std::ranges::move(Right.mRelations, std::back_inserter(mRelations));

        // Adjust indexes for the compounds from the right.
        for (int i = 0; i < mCompounds.size(); i++)
        {
            mCompounds[i].ShiftIndexes(0, LeftRelationSize);
        }

        if (mCompounds.back().HasConnective())
        {
            mCompounds.emplace_back(Connective, LeftPredicate, CPredicate(EPredicate::compound, mCompounds.size() - 1));
        }
        else
        {
            mCompounds.back().Connective = Connective;
            mCompounds.back().Left = LeftPredicate;
        }
    }
    else 
    {
        const int LeftRelationSize = static_cast<int>(mRelations.size());
        const int LeftCompoundSize = static_cast<int>(mCompounds.size());
        std::ranges::move(Right.mRelations, std::back_inserter(mRelations));
        std::ranges::move(Right.mCompounds, std::back_inserter(mCompounds));

        // Adjust indexes for the compounds from the right.
        for (int i = LeftCompoundSize; i < mCompounds.size(); i++)
        {
            mCompounds[i].ShiftIndexes(LeftCompoundSize, LeftRelationSize);
        }

        if (mCompounds.back().HasConnective())
        {
            mCompounds.emplace_back(Connective, CPredicate(EPredicate::compound, LeftCompoundSize - 1), CPredicate(EPredicate::compound, mCompounds.size() - 1));
        }
        else
        {
            mCompounds.back().Connective = Connective;
            mCompounds.back().Left = CPredicate(EPredicate::compound, LeftCompoundSize - 1);
        }
    }
}

void CFormula::ReplaceFact(const CFact& Fact, const COperand& Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CFormula::ReplaceFact(int FactIndex, const COperand& Replacement)
{
    for (CRelation& Relation : mRelations)
    {
        Relation.GetLeft().ReplaceFact(FactIndex, Replacement);
        Relation.GetRight().ReplaceFact(FactIndex, Replacement);
    }
}

void CFormula::ReplaceFact(const CFact& Fact, const CExpression& Replacement)
{
    ReplaceFact(Fact.GetIndex(), Replacement);
}

void CFormula::ReplaceFact(int FactIndex, const CExpression& Replacement)
{
    if (Replacement.IsEmpty())
    {
        return;
    }

    for (CRelation& Relation : mRelations)
    {
        Relation.GetLeft().ReplaceFact(FactIndex, Replacement);
        Relation.GetRight().ReplaceFact(FactIndex, Replacement);
    }
}

std::unordered_map<int, int> CFormula::ReplaceCompound(int CompoundIndex, CFormula&& Replacement)
{
    assert(CompoundIndex >= 0 && CompoundIndex < mCompounds.size());
    assert(Replacement.mRelations.size() == 0); // Relation migration is not supported because it's hard to tell which formula references these relations.

    if (Replacement.IsEmpty())
    {
        return {};
    }

    // Check whether the first compound of the replacement is valid.
    assert(ValidateCompound(Replacement.mCompounds.front()));
    // Replace the specified compound with the first compound of the replacement.
    mCompounds[CompoundIndex] = Replacement.mCompounds.front();

    const int ReplacementCompoundCount = Replacement.GetCompoundCount();
    if (ReplacementCompoundCount == 1)
    {
        return {};
    }

    // Insert the replacement compounds except the first one.
    mCompounds.insert(mCompounds.begin() + CompoundIndex + 1, Replacement.mCompounds.begin() + 1, Replacement.mCompounds.end());

    std::unordered_map<int, int> CompoundIndexMap;
    CompoundIndexMap.emplace(CompoundIndex, CompoundIndex + ReplacementCompoundCount - 1); // Map the replaced compound to the last compound of the inserted replacement.

    int CompoundOffset = ReplacementCompoundCount - 1;
    for (int i = CompoundIndex + ReplacementCompoundCount; i < mCompounds.size(); i++)
    {
        CompoundIndexMap.emplace(i - CompoundOffset, i);

        CPredicate& Left = mCompounds[i].Left;
        CPredicate& Right = mCompounds[i].Right;
        if (Left.IsCompound() && Left.GetCompoundIndex() >= CompoundIndex)
        {
            Left.ShiftIndexes(CompoundOffset, 0);
        }
        if (Right.IsCompound() && Right.GetCompoundIndex() >= CompoundIndex)
        {
            Right.ShiftIndexes(CompoundOffset, 0);
        }
    }

    return CompoundIndexMap;
}

void CFormula::RemoveUnusedCompoundsAndRelations()
{
    std::vector<int> UsedCompoundIndexes;
    std::vector<int> UsedRelationIndexes;
    GetUsedCompoundAndRelationIndexes(UsedCompoundIndexes, UsedRelationIndexes);

    std::vector<bool> CompoundsUsed(mCompounds.size(), false);
    for (int i = 0; i < UsedCompoundIndexes.size(); i++)
    {
        int CompoundIndex = UsedCompoundIndexes[i];
        CompoundsUsed[CompoundIndex] = true;
    }

    std::vector<bool> RelationsUsed(mRelations.size(), false);
    for (int i = 0; i < UsedRelationIndexes.size(); i++)
    {
        int RelationIndex = UsedRelationIndexes[i];
        RelationsUsed[RelationIndex] = true;
    }

    std::vector<int> NewCompoundIndexes(mCompounds.size());
    for (int i = 0; i < NewCompoundIndexes.size(); i++)
    {
        NewCompoundIndexes[i] = i;
    }

    std::vector<int> NewRelationIndexes(mRelations.size());
    for (int i = 0; i < NewRelationIndexes.size(); i++)
    {
        NewRelationIndexes[i] = i;
    }

    for (int i = static_cast<int>(mCompounds.size()) - 1; i >= 0; i--)
    {
        if (CompoundsUsed[i])
        {
            continue;
        }

        mCompounds.erase(mCompounds.begin() + i);

        // The current compound will be deleted, so subsequent compound indexes need to be updated.
        for (int IndexOfCompoundIndexes = i + 1; IndexOfCompoundIndexes < NewCompoundIndexes.size(); IndexOfCompoundIndexes++)
        {
            NewCompoundIndexes[IndexOfCompoundIndexes]--;
        }
    }

    for (int i = static_cast<int>(mRelations.size()) - 1; i >= 0; i--)
    {
        if (RelationsUsed[i])
        {
            continue;
        }

        mRelations.erase(mRelations.begin() + i);

        // The current relation will be deleted, so subsequent relation indexes need to be updated.
        for (int IndexOfRelationIndexes = i + 1; IndexOfRelationIndexes < NewRelationIndexes.size(); IndexOfRelationIndexes++)
        {
            NewRelationIndexes[IndexOfRelationIndexes]--;
        }
    }

    auto UpdateIndex = [&](CPredicate& Predicate)
    {
        if (Predicate.IsAtomic())
        {
            int NewIndex = NewRelationIndexes[Predicate.GetRelationIndex()];
            Predicate.SetRelationIndex(NewIndex);
        }
        else if (Predicate.IsCompound())
        {
            int NewIndex = NewCompoundIndexes[Predicate.GetCompoundIndex()];
            Predicate.SetCompoundIndex(NewIndex);
        }
    };

    for (int i = 0; i < mCompounds.size(); i++)
    {
        SCompound& Compound = mCompounds[i];
        UpdateIndex(Compound.Left);
        UpdateIndex(Compound.Right);
    }
}

CFormula CFormula::GetRearranged(const CFactDefinition& Definition) const
{
    CFormula Return = *this;
    Return.Rearrange(Definition);
    return Return;
}

void CFormula::Rearrange(const CFactDefinition& Definition)
{
    // Apply De Morgan's laws before rearranging these relational expressions, as this pushes the negation inside and inverts their comparers.
    ApplyDeMorgansLaws();

    for (CRelation& Relation : mRelations)
    {
        Relation.Rearrange();
    }

    ReduceUnconditionalAndUnsatisfiableInequalities();
    RemoveRedundantBoolsAndAtomicCompounds(Definition);
    RearrangeConjunctionAndDisjunction(Definition.GetTolerance());
    RemoveRedundantBoolsAndAtomicCompounds(Definition);
    RemoveUnusedCompoundsAndRelations();

    // TODO: Apply logical laws to transform equivalent logical formulas into the same form, such as the idempotent, tautology, and absorption laws.

    assert(GetInvalidCompoundIndexes().empty());
    assert(CountUnusedCompoundsAndRelations() == std::make_pair(0, 0));
    assert(CountUsedAtomicCompounds() == 0);
}

void CFormula::RearrangeConjunctionAndDisjunction(CNumber Tolerance)
{
    // Unary compounds can be references to atomic or compound formulas, and the latter will make the first pass hard to implement.
    // So, RemoveRedundantBoolsAndAtomicCompounds must be called beforehand to avoid the issue.
    assert(CountUsedAtomicCompounds() == 0);

    // First, sort the relations so that equivalent formulas will have relations in the same order.
    // Thus, when rearranging atomic formulas in repeated logical operations, we just need to sort the relation indexes.
    SortRelations();

    auto CollectRepeatedOperation = [](SRepeatedLogicalOperation& RepeatedOperation, const CPredicate& Predicate)
    {
        if (Predicate.IsCompound())
        {
            RepeatedOperation.CompoundIndexes.push_back(Predicate.GetCompoundIndex());
        }
        else if (Predicate.IsAtomic())
        {
            RepeatedOperation.RelationIndexes.push_back(Predicate.GetRelationIndex());
        }
        else if (Predicate.IsBool())
        {
            if (RepeatedOperation.Result.IsUnknown())
            {
                RepeatedOperation.Result = Predicate.GetBool();
            }
            else
            {
                RepeatedOperation.Result = RepeatedOperation.Connective(RepeatedOperation.Result, ETriState(Predicate.GetBool()));
            }
        }
    };

    // First PASS: Rearrange operands in commutative compounds in a specific order and collect repeated operations.
    std::vector<SRepeatedLogicalOperation> RepeatedOps;
    for (int CompoundIndex = static_cast<int>(mCompounds.size()) - 1; CompoundIndex >= 0; CompoundIndex--)
    {
        SCompound& Compound = mCompounds[CompoundIndex];
        if (!Compound.Connective.IsCommutative())
        {
            continue;
        }
        assert(Compound.Connective.GetArity() == 2);

        CPredicate& Left = Compound.Left;
        CPredicate& Right = Compound.Right;
        bool ToAddRepeatedOp = false;
        auto CheckReferencedConnectiveRepeated = [&](CPredicate& Predicate)
        {
            int RefCompoundIndex = Predicate.GetCompoundIndex();
            if (mCompounds[RefCompoundIndex].Connective == Compound.Connective)
            {
                ToAddRepeatedOp = true;
            }
        };

        // Rearrange the two operands in order: compound, relation, and Boolean constant.
        if (Left.IsBool())
        {
            if (Right.IsAtomic())
            {
                std::swap(Left, Right);
            }
            else if (Right.IsCompound())
            {
                CheckReferencedConnectiveRepeated(Right);
                std::swap(Left, Right);
            }
            // Keep the case of two Boolean constants intact, as they will be merged into one in repeated operations.
        }
        else if (Left.IsAtomic())
        {
            if (Right.IsAtomic())
            {
                if (Left.GetRelationIndex() > Right.GetRelationIndex())
                {
                    std::swap(Left, Right);
                }

                // Assuming the two relational expressions are transposable and already rearranged, their right sides should be constant terms, with the remaining terms on the left.
                // If both left sides are the same, these relational expressions are compound inequalities and could be simplified.
                // Hence, they will be added to a repeated logical operation for later simplification.
                const CRelation& LeftRelation = mRelations[Left.GetRelationIndex()];
                const CRelation& RightRelation = mRelations[Right.GetRelationIndex()];
                if ((LeftRelation.GetRight().IsOnlyOneConstant()) && RightRelation.GetRight().IsOnlyOneConstant())
                {
                    if (LeftRelation.GetLeft().IsEquivalent(RightRelation.GetLeft()))
                    {
                        ToAddRepeatedOp = true;
                    }
                }
            }
            else if (Right.IsCompound())
            {
                CheckReferencedConnectiveRepeated(Right);
                std::swap(Left, Right);
            }
        }
        else if (Left.IsCompound())
        {
            if (Right.IsCompound())
            {
                if (Left.GetCompoundIndex() == Right.GetCompoundIndex())
                {
                    CheckReferencedConnectiveRepeated(Right);
                }
                else
                {
                    // If the two operands reference different compoounds, then they are added to a repeated operation so that they can be sorted in the second pass.
                    ToAddRepeatedOp = true;
                }
            }
            else
            {
                CheckReferencedConnectiveRepeated(Left);
            }
        }

        // Replace the current compound in repeated operations with its operands.
        bool AlreadyInRepeatedOp = false;
        for (int RepeatedOpIndex = 0; RepeatedOpIndex < RepeatedOps.size(); RepeatedOpIndex++)
        {
            SRepeatedLogicalOperation& RepeatedOp = RepeatedOps[RepeatedOpIndex];
            if (Compound.Connective == RepeatedOp.Connective) // Check if the current connective matches the repeated operation.
            {
                for (int OpIndexOfRepeatedOp = static_cast<int>(RepeatedOp.CompoundIndexes.size()) - 1; OpIndexOfRepeatedOp >= 0; OpIndexOfRepeatedOp--)
                {
                    if (CompoundIndex == RepeatedOp.CompoundIndexes[OpIndexOfRepeatedOp]) // Check if this compound is referenced by the current repeated operation.
                    {
                        RepeatedOp.CompoundIndexes.erase(RepeatedOp.CompoundIndexes.begin() + OpIndexOfRepeatedOp);
                        CollectRepeatedOperation(RepeatedOp, Compound.Left);
                        CollectRepeatedOperation(RepeatedOp, Compound.Right);
                        AlreadyInRepeatedOp = true;
                    }
                }
            }
        }

        // Add a new repeated operation starting with the current compound if no corresponding one exists.
        if (ToAddRepeatedOp && !AlreadyInRepeatedOp)
        {
            SRepeatedLogicalOperation& RepeatedOp = RepeatedOps.emplace_back(Compound.Connective, CompoundIndex);
            CollectRepeatedOperation(RepeatedOp, Compound.Left);
            CollectRepeatedOperation(RepeatedOp, Compound.Right);
        }
    }

    // Second PASS:
    // 1. Rearrange the operands of the repeated operations in order.
    // 2. Replace the source compounds with the corresponding repeated operations and update compound indexes.
    // 3. Call RemoveRedundantBoolsAndAtomicCompounds to remove predicates that can be reduced to truth values (later in Rearrange).
    // 4. Call RemoveUnusedCompoundsAndRelations to remove replaced compounds and relations (later in Rearrange).
    // TODO: Simplify compound inequalities combined with both logical AND & OR like: (X>1∧X<2) ∨ (X≳2∧X<3) ≡ X>1∧X<3
    assert(std::ranges::is_sorted(RepeatedOps, SRepeatedLogicalOperation::IsSourceCompoundIndexGreaterThanAnother));
    for (int RepeatedOpIndex = static_cast<int>(RepeatedOps.size()) - 1; RepeatedOpIndex >= 0; RepeatedOpIndex--)
    {
        SRepeatedLogicalOperation& RepeatedOp = RepeatedOps[RepeatedOpIndex];
        std::vector<int>& RelationIndexes = RepeatedOp.RelationIndexes;
        std::vector<int>& CompoundIndexes = RepeatedOp.CompoundIndexes;
        assert(RepeatedOp.Connective.IsCommutative());
        assert(RepeatedOp.SourceCompoundIndex >= 0);

        // Simplify the compound inequalities in these atomic formulas if available.
        CFormula SubFormula;
        if (RepeatedOp.RelationIndexes.size() > 1)
        {
            SubFormula = CombineCompoundInequalities(RepeatedOp.Connective, RelationIndexes, Tolerance);
            ETriState Result = SubFormula.TryGetTheOnlyBool();
            if (Result.IsNo() && RepeatedOp.Connective.IsAND())
            {
                RepeatedOp.Result = false;
                RelationIndexes.clear();
                CompoundIndexes.clear();
                SubFormula.Clear();
            }
            else if (Result.IsYes() && RepeatedOp.Connective.IsOR())
            {
                RepeatedOp.Result = true;
                RelationIndexes.clear();
                CompoundIndexes.clear();
                SubFormula.Clear();
            }
            else 
            {
                EConnective CommonConnective = SubFormula.GetCommonConnective();
                assert(CommonConnective.IsValid());
                assert(CommonConnective.IsNil() || CommonConnective.IsCommutative());
                if (CommonConnective.IsNil() || CommonConnective == RepeatedOp.Connective)
                {
                    // Replace the referenced relation indexes.
                    RelationIndexes.resize(SubFormula.mRelations.size());
                    for (int i = 0; i < SubFormula.mRelations.size(); i++)
                    {
                        RelationIndexes[i] = static_cast<int>(mRelations.size()) + i;
                    }

                    std::ranges::move(SubFormula.mRelations, std::back_inserter(mRelations)); // Add the relations from the combined formula.
                    SortRelations(RelationIndexes); // Sort the newly-added relations for ease of comparison.
                    SubFormula.Clear();
                }
                else
                {
                    // If the connective of the simplified compound inequalities is different from that of the repeated operation,
                    // then clear the relation references and combine the resulting formula later instead.
                    RelationIndexes.clear();
                }
            }
        }

        // Sort referenced compounds.
        SortSubFormulas(CompoundIndexes);

        if (CompoundIndexes.size() == 2 && RelationIndexes.empty()
            && (RepeatedOp.Result.IsUnknown() || RepeatedOp.Result == RepeatedOp.Connective.GetIdentity()))
        {
            // With only two compound references, we just need to rearrange them.
            SCompound& Compound = mCompounds[RepeatedOp.SourceCompoundIndex];
            Compound.Left.SetCompoundIndex(CompoundIndexes[0]);
            Compound.Right.SetCompoundIndex(CompoundIndexes[1]);
        }
        else
        {
            CFormula RepeatedOpFormula = RepeatedOp.ToFormula();
            if (!SubFormula.IsEmpty())
            {
                // Combine the simplified compound inequalities and rearrange the resulting formula.
                assert(SubFormula.GetCommonConnective() != RepeatedOp.Connective);
                RepeatedOpFormula.Append(RepeatedOp.Connective, std::move(SubFormula));
                RepeatedOpFormula.RearrangeConjunctionAndDisjunction(Tolerance);
                RepeatedOpFormula.RemoveUnusedCompoundsAndRelations();
                // Move relations and shift their indexes before replacing the source compound.
                RepeatedOpFormula.ShiftIndexes(0, static_cast<int>(mRelations.size()));
                std::ranges::move(RepeatedOpFormula.mRelations, std::back_inserter(mRelations));
                RepeatedOpFormula.mRelations.clear();
            }

            auto CompoundIndexMap = ReplaceCompound(RepeatedOp.SourceCompoundIndex, std::move(RepeatedOpFormula));
            // Update only the repeated operations preceding the current one, since the others have been used.
            for (int i = RepeatedOpIndex - 1; i >= 0; i--)
            {
                RepeatedOps[i].UpdateCompoundIndexes(CompoundIndexMap);
            }
        }
    }
}

CFormula CFormula::CombineCompoundInequalities(EConnective Connective, const std::vector<int>& RelationIndexes, CNumber Tolerance)
{
    // PREREQUISITE: Constant terms are moved to the right side and the rest to the left in the relational expressions.
    // This function not only combines compound inequalities, but also, in a sense, fulfills the laws of idempotence, contradiction, and excluded middle for atomic formulas. 
    // However, it is imperfect due to incomplete expression rearrangement and floating-point imprecision.

    assert(Connective.IsAND() || Connective.IsOR());

    struct SIndexedRange
    {
        int Index = InvalidIndex;
        CRange Range;
    };

    // Build a relation map to merge the ranges of compound inequalities.
    std::unordered_map<std::string, SIndexedRange> RelationMap;
    RelationMap.reserve(RelationIndexes.size());
    for (int i = 0; i < RelationIndexes.size(); i++)
    {
        int RelationIndex = RelationIndexes[i];
        CRelation& Relation = mRelations[RelationIndex];
        // Assuming the relational expression is already rearranged, the right side should be a constant term, with the remaining terms on the left.
        assert(Relation.GetRight().IsOnlyOneConstant());
        std::string LeftText = Relation.GetLeft().ToString();
        auto it = RelationMap.find(LeftText);
        if (it == RelationMap.end())
        {
            CRange Range(Relation.GetComparer(), Relation.GetRight().GrabTheOnlyConstant(), Tolerance);
            assert(!Range.IsEmpty());
            RelationMap.emplace(LeftText, SIndexedRange{RelationIndex, std::move(Range)});
        }
        else
        {
            if (Connective.IsAND())
            {
                it->second.Range.Intersect(Relation.GetComparer(), Relation.GetRight().GrabTheOnlyConstant());
                if (it->second.Range.IsEmpty())
                {
                    return CFormula(false); // There is one or more conflicts between the relational expressions.
                }
            }
            else if (Connective.IsOR())
            {
                it->second.Range.Union(Relation.GetComparer(), Relation.GetRight().GrabTheOnlyConstant());
                assert(!it->second.Range.IsEmpty());
            }
            else
            {
                assert(!"Unsupported connective type");
            }
        }
    }

    // Convert the ranges back to compound inequalities.
    CFormula Return;
    for (auto& [Text, IndexedRange] : RelationMap)
    {
        auto& [Index, Range] = IndexedRange;
        Range.Sort();

        CFormula SubFormula;
        if (!BuildFormula(SubFormula, Connective, mRelations[Index].GetLeft(), Range))
        {
            if (Connective.IsAND())
            {
                // The range of compound inequalities is stored as a union of intervals.
                // So, it may not be able to expressed as just a double inequality in some edge cases.
                // However, we can always combine inequalities with logical OR instead.
                bool Ok = BuildFormula(SubFormula, EConnective::logicalOR, mRelations[Index].GetLeft(), Range);
                assert(Ok);
            }
        }

        ETriState Result = SubFormula.TryGetTheOnlyBool();
        if (Result.IsNo())
        {
            if (Connective.IsAND())
            {
                return CFormula(false);
            }
        }
        else if (Result.IsYes())
        {
            if (Connective.IsOR())
            {
                return CFormula(true);
            }
        }
        else
        {
            Return.Append(Connective, std::move(SubFormula));
        }
    }

    return Return;
}

void CFormula::ShiftIndexes(int CompoundOffset, int RelationOffset)
{
    for (SCompound& Compound : mCompounds)
    {
        Compound.ShiftIndexes(CompoundOffset, RelationOffset);
    }
}

void CFormula::SortRelations()
{
    std::vector<SIndexedSizeAndText> RelationTuples;
    RelationTuples.reserve(mRelations.size());
    std::vector<int> SwappedIndexes(mRelations.size()); // Original relation indexes after swapping
    std::vector<int> NewIndexes(mRelations.size()); // Mapping of original relation indexes to new ones
    for (int i = 0; i < mRelations.size(); i++)
    {
        RelationTuples.emplace_back(i, mRelations[i].AddUpArity(), mRelations[i].ToString());
        SwappedIndexes[i] = i;
        NewIndexes[i] = i;
    }

    std::ranges::sort(RelationTuples, &SIndexedSizeAndText::Compare);

    // Rearrange relations according to the sorted order.
    for (int i = 0; i < mRelations.size(); i++)
    {
        int SourceIndex = SwappedIndexes[i];
        int TargetIndex = RelationTuples[i].Index;
        if (SourceIndex == TargetIndex)
        {
            continue;
        }

        int NewTargetIndex = NewIndexes[TargetIndex];
        std::swap(SwappedIndexes[i], SwappedIndexes[NewTargetIndex]);
        std::swap(mRelations[i], mRelations[NewTargetIndex]);
        std::swap(NewIndexes[SourceIndex], NewIndexes[TargetIndex]);
    }

    // Sanity check
    for (int i = 0; i < mRelations.size(); i++)
    {
        assert(RelationTuples[i].Index == SwappedIndexes[i]);
        assert(NewIndexes[SwappedIndexes[i]] == i);
    }

    // Update referenced relation indexes in compounds.
    for (SCompound& Compound : mCompounds)
    {
        CPredicate& Left = Compound.Left;
        CPredicate& Right = Compound.Right;
        if (Left.IsAtomic())
        {
            Left.SetRelationIndex(NewIndexes[Left.GetRelationIndex()]);
        }
        if (Right.IsAtomic())
        {
            Right.SetRelationIndex(NewIndexes[Right.GetRelationIndex()]);
        }
    }
}

void CFormula::SortRelations(std::vector<int>& RelationIndexes) const
{
    if (RelationIndexes.size() <= 1)
    {
        return;
    }

    std::vector<SIndexedText> RelationTuples;
    RelationTuples.reserve(RelationIndexes.size());

    for (int i = 0; i < RelationIndexes.size(); i++)
    {
        const int RelationIndex = RelationIndexes[i];
        const CRelation& Relation = mRelations[RelationIndex];
        RelationTuples.emplace_back(RelationIndex, Relation.ToString());
    }

    std::ranges::sort(RelationTuples, &SIndexedText::Compare);

    for (int i = 0; i < RelationIndexes.size(); i++)
    {
        RelationIndexes[i] = RelationTuples[i].Index;
    }
}

void CFormula::SortSubFormulas(std::vector<int>& StartCompoundIndexes) const
{
    if (StartCompoundIndexes.size() <= 1)
    {
        return;
    }

    std::vector<SIndexedSizeAndText> SubFormulaTuples;
    SubFormulaTuples.reserve(StartCompoundIndexes.size());

    for (int i = 0; i < StartCompoundIndexes.size(); i++)
    {
        int StartCompoundIndex = StartCompoundIndexes[i];
        std::vector<int> UsedCompoundIndexes = GetUsedCompoundIndexes(StartCompoundIndex);
        std::ranges::sort(UsedCompoundIndexes);
        SubFormulaTuples.emplace_back(StartCompoundIndex, AddUpArity(UsedCompoundIndexes), ToString(UsedCompoundIndexes));
    }

    std::ranges::sort(SubFormulaTuples, &SIndexedSizeAndText::Compare);

    for (int i = 0; i < StartCompoundIndexes.size(); i++)
    {
        StartCompoundIndexes[i] = SubFormulaTuples[i].Index;
    }
}

void CFormula::ApplyDeMorgansLaws() 
{
    // PREREQUISITE: This implementation is based on the premise that each compound and relation is referenced no more than once in this formula.
    // TODO: Rework the code to remove the prerequisite.
    assert(CountMostUsedCompound().second <= 1);
    assert(CountMostUsedRelation().second <= 1);

    std::vector<bool> Negated(mCompounds.size(), false); // Temporary flags to mark if a compound is negated when applying De Morgan's laws.
    auto NegatePredicate = [&](const CPredicate& Predicate)
    {
        if (Predicate.IsAtomic())
        {
            mRelations[Predicate.GetRelationIndex()].Negate();
        }
        else if (Predicate.IsCompound())
        {
            assert(!Negated[Predicate.GetCompoundIndex()]); // Verify that the prerequisite is not compromised.
            Negated[Predicate.GetCompoundIndex()] = true;
        }
    };
    auto NegateCompound = [&](const SCompound& Compound)
    {
        NegatePredicate(Compound.Left);
        NegatePredicate(Compound.Right);
    };

    for (int i = static_cast<int>(mCompounds.size()) - 1; i >= 0; i--)
    {
        SCompound& Compound = mCompounds[i];
        bool Negative = Negated[i] ^ Compound.Connective.IsNOT();
        if (Negative)
        {
            if (Compound.Connective.IsNOT())
            {
                Compound.Connective = EConnective::nil;
                NegateCompound(Compound);
            }
            else if (Compound.Connective.IsAND())
            {
                Compound.Connective = EConnective::logicalOR;
                NegateCompound(Compound);
            }
            else if (Compound.Connective.IsOR())
            {
                Compound.Connective = EConnective::logicalAND;
                NegateCompound(Compound);
            }
        }
        else if (Compound.Connective.IsNOT())
        {
            Compound.Connective = EConnective::nil;
        }
    }
}

void CFormula::ReduceUnconditionalAndUnsatisfiableInequalities()
{
    // Reduce unconditional inequalities to true and unsatisfiable inequalities to false.
    // Call RemoveRedundantBoolsAndAtomicCompounds later to remove the redundant Boolean constants generated by this function.

    std::vector<ETriState> RelationResults;
    RelationResults.resize(mRelations.size(), ETriState::unknown);
    for (int RelationIndex = 0; RelationIndex < mRelations.size(); RelationIndex++)
    {
        const CRelation& Relation = mRelations[RelationIndex];
        if (!Relation.GetRight().IsOnlyOneConstant())
        {
            continue;
        }

        RelationResults[RelationIndex] = Relation.UnconditionallyEvaluate();
    }

    auto Reduce = [&](CPredicate& Predicate)
    {
        if (!Predicate.IsAtomic())
        {
            return;
        }

        ETriState Result = RelationResults[Predicate.GetRelationIndex()];
        if (Result.IsNo())
        {
            Predicate.SetBool(false);
        }
        else if (Result.IsYes())
        {
            Predicate.SetBool(true);
        }
    };

    for (SCompound& Compound : mCompounds)
    {
        Reduce(Compound.Left);
        Reduce(Compound.Right);
    }
}

void CFormula::RemoveRedundantBoolsAndAtomicCompounds(const CFactDefinition& Definition)
{
    if (mCompounds.empty())
    {
        return;
    }

    std::vector<int> NewRelationIndexes(mRelations.size());
    std::vector<int> NewCompoundIndexes(mCompounds.size());
    for (int i = 0; i < mRelations.size(); i++)
    {
        NewRelationIndexes[i] = i;
    }
    for (int i = 0; i < mCompounds.size(); i++)
    {
        NewCompoundIndexes[i] = i;
    }

    std::vector<bool> RelationsToRemove(mRelations.size(), false);
    auto MarkRelationToRemove = [&](int RelationIndex)
    {
        if (RelationsToRemove[RelationIndex]) // Check if it's already marked.
        {
            return;
        }

        RelationsToRemove[RelationIndex] = true;

        // The current relation will be deleted, so subsequent relation indexes need to be updated.
        for (int IndexOfIndexes = RelationIndex + 1; IndexOfIndexes < NewRelationIndexes.size(); IndexOfIndexes++)
        {
            NewRelationIndexes[IndexOfIndexes]--;
        }
    };

    // Evaluate relations and build new index mappings after removing unused relations.
    std::vector<ETriState> RelationResults(mRelations.size(), ETriState::unknown);
    for (int RelationIndex = 0; RelationIndex < mRelations.size(); RelationIndex++)
    {
        RelationResults[RelationIndex] = mRelations[RelationIndex].Evaluate(Definition.GetEmptyState());
        if (!RelationResults[RelationIndex].IsUnknown())
        {
            MarkRelationToRemove(RelationIndex);
        }
    }

    std::vector<ETriState> CompoundResults(mCompounds.size(), ETriState::unknown);
    auto EvaluatePredicate = [&](CPredicate& Predicate)
    {
        ETriState Result = ETriState::unknown;

        if (Predicate.IsBool())
        {
            Result = Predicate.GetBool();
        }
        else if (Predicate.IsAtomic())
        {
            Result = RelationResults[Predicate.GetRelationIndex()];
        }
        else if (Predicate.IsCompound())
        {
            int RefCompoundIndex = Predicate.GetCompoundIndex();
            Result = CompoundResults[RefCompoundIndex];
            if (Result.IsUnknown())
            {
                if (!mCompounds[RefCompoundIndex].HasConnective())
                {
                    // If the referenced compound has no connective, let the output predicate be the only operand.
                    // Hence, we can avoid redundant compounds and references.
                    Predicate = mCompounds[RefCompoundIndex].Right;
                }
            }
        }

        return Result;
    };

    // Evaluate compounds and combine them by identity laws, or remove them by domination laws.
    std::vector<bool> CompoundsToRemove(mCompounds.size(), false);
    for (int CompoundIndex = 0; CompoundIndex < mCompounds.size(); CompoundIndex++)
    {
        SCompound& Compound = mCompounds[CompoundIndex];
        ETriState LeftValue = EvaluatePredicate(Compound.Left);
        ETriState RightValue = EvaluatePredicate(Compound.Right);
        CompoundResults[CompoundIndex] = Compound.Connective(LeftValue, RightValue);

        if (CompoundResults[CompoundIndex].IsUnknown())
        {
            if (!Compound.HasConnective())
            {
                // Mark a compound without a connective for removal, because it has only a predicate and is redundant. 
                CompoundsToRemove[CompoundIndex] = true;

                if (Compound.Right.IsCompound())
                {
                    assert(mCompounds[Compound.Right.GetCompoundIndex()].HasConnective());
                    NewCompoundIndexes[CompoundIndex] = NewCompoundIndexes[Compound.Right.GetCompoundIndex()];
                }
            }
            else if (Compound.Connective.GetArity() > 1)
            {
                // If only one side is constant, set the current compound to the other one.
                // This way, subsequent compounds referencing the current compound will reference that instead.
                // Hence, we can avoid redundant compounds and references.
                if (LeftValue.IsUnknown() && !RightValue.IsUnknown()) // Left only
                {
                    Compound.SetTo(Compound.Left);
                    CompoundsToRemove[CompoundIndex] = true;
                }
                else if (!LeftValue.IsUnknown() && RightValue.IsUnknown()) // Right only
                {
                    Compound.SetTo(Compound.Right);
                    CompoundsToRemove[CompoundIndex] = true;
                }
            }
        }
        else
        {
            // The result of the current compound is constant and redundant, so mark it and the referenced relations for removal.
            if (Compound.Left.IsAtomic())
            {
                MarkRelationToRemove(Compound.Left.GetRelationIndex());
            }
            if (Compound.Right.IsAtomic())
            {
                MarkRelationToRemove(Compound.Right.GetRelationIndex());
            }

            CompoundsToRemove[CompoundIndex] = true;        
        }

        if (CompoundsToRemove[CompoundIndex])
        {
            // The current compound will be deleted, so subsequent compound indexes need to be updated.
            for (int IndexOfIndexes = CompoundIndex + 1; IndexOfIndexes < NewCompoundIndexes.size(); IndexOfIndexes++)
            {
                NewCompoundIndexes[IndexOfIndexes]--;
            }
        }
    }

    ETriState Result = CompoundResults.back();
    if (Result == ETriState::yes)
    {
        SetTo(true);
    }
    else if (Result == ETriState::no)
    {
        SetTo(false);
    }
    else
    {
        // Remove constant relations.
        for (int i = static_cast<int>(mRelations.size()) - 1; i >= 0; i--)
        {
            if (RelationsToRemove[i])
            {
                mRelations.erase(mRelations.begin() + i);
            }
        }

        // Remove unused compounds except the last one.
        for (int i = static_cast<int>(mCompounds.size()) - 2; i >= 0; i--)
        {
            if (CompoundsToRemove[i])
            {
                mCompounds.erase(mCompounds.begin() + i);
            }
        }

        auto UpdateIndex = [&](CPredicate& Predicate)
        {
            if (Predicate.IsAtomic())
            {
                Predicate.SetRelationIndex(NewRelationIndexes[Predicate.GetRelationIndex()]);
                assert(Predicate.GetRelationIndex() < mRelations.size());
            }
            else if (Predicate.IsCompound())
            {
                Predicate.SetCompoundIndex(NewCompoundIndexes[Predicate.GetCompoundIndex()]);
                assert(Predicate.GetCompoundIndex() < mCompounds.size());
            }
        };

        // Update the compound and relation indexes.
        for (int i = 0; i < mCompounds.size(); i++)
        {
            UpdateIndex(mCompounds[i].Left);
            UpdateIndex(mCompounds[i].Right);
        }
    }
}

void CFormula::SplitInto(std::vector<CFormula>& oFormulas)
{
    SplitInto(oFormulas, static_cast<int>(mCompounds.size()) - 1);
}

void CFormula::SplitInto(std::vector<CFormula>& oFormulas, int StartCompoundIndex)
{
    assert(StartCompoundIndex >= 0 && StartCompoundIndex < mCompounds.size());

    std::vector<int> CompoundIndexes;
    CompoundIndexes.reserve(StartCompoundIndex + 1);
    auto AddFormula = [&](const CPredicate& Predicate)
    {
        switch (Predicate.GetType())
        {
        case EPredicate::boolean:
            oFormulas.emplace_back(Predicate.GetBool());
            break;
        case EPredicate::atomic:
            oFormulas.emplace_back(mRelations[Predicate.GetRelationIndex()]);
            break;
        case EPredicate::compound:
            CompoundIndexes.push_back(Predicate.GetCompoundIndex());
            break;
        }
    };

    CompoundIndexes.push_back(StartCompoundIndex);
    for (int i = 0; i < CompoundIndexes.size(); i++)
    {
        const int CompoundIndex = CompoundIndexes[i];
        const SCompound& Compound = mCompounds[CompoundIndex];
        if (!Compound.HasConnective())
        {
            AddFormula(Compound.Right);
        }
        else if (Compound.Connective.IsAND())
        {
            AddFormula(Compound.Left);
            AddFormula(Compound.Right);
        }
        else
        {
            oFormulas.emplace_back(GetSubFormula(CompoundIndex));
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////

