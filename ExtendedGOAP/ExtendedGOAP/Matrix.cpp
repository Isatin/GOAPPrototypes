// Copyright 2026 Isaac Hsu

#include <algorithm>
#include <cassert>
#include <span>

#include "Matrix.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
const CMatrix CMatrix::Empty;
const CMatrix CMatrix::InvalidType(EVariantError::invalidType);
const CMatrix CMatrix::InvalidSize(EVariantError::invalidSize);

CMatrix::~CMatrix()
{
    DestructEntries();
}

CMatrix::CMatrix(EVariantError Error)
{
    mType = EVariantType::matrix;
    mError = Error;
    mRowCount = 0;
    mColumnCount = 0;
    ConstructEntries();
}

CMatrix::CMatrix(const CMatrix& Another)
{
    mType = Another.mType;
    mError = Another.mError;
    mRowCount = Another.mRowCount;
    mColumnCount = Another.mColumnCount;
    ConstructEntries();

    if (mRowCount > 0)
    {
        assert(Another.mColumnCount > 0);
        assert(Another.mCapacity >= Another.mRowCount * Another.mColumnCount);
        assert(Another.mEntries.get());

        ResizeEntries(Another.mCapacity); 
        std::ranges::copy(std::span(Another.mEntries.get(), mRowCount * mColumnCount), mEntries.get());
    }
}

CMatrix::CMatrix(CMatrix&& Another) noexcept
{
    mType = Another.mType;
    mError = Another.mError;
    mRowCount = Another.mRowCount;
    mColumnCount = Another.mColumnCount;
    ConstructEntries();

    if (mRowCount > 0)
    {
        assert(Another.mColumnCount > 0);
        assert(Another.mCapacity >= Another.mRowCount * Another.mColumnCount);
        assert(Another.mEntries.get());

        mEntries = std::move(Another.mEntries);
        mCapacity = Another.mCapacity;
    }
}

CMatrix::CMatrix(CNumber Value, int RowCount, int ColumnCount)
{
    assert(RowCount > 0 && RowCount <= std::numeric_limits<BEntrySize>::max());
    assert(ColumnCount > 0 && ColumnCount <= std::numeric_limits<BEntrySize>::max());

    mType = EVariantType::matrix;
    mError = EVariantError::none;
    mRowCount = RowCount;
    mColumnCount = ColumnCount;

    ConstructEntries();
    ResizeEntries(RowCount * ColumnCount);
    std::fill(mEntries.get(), mEntries.get() + mCapacity, Value);
}

CMatrix::CMatrix(std::initializer_list< std::initializer_list<CNumber> > Source)
{
    assert(Source.size() <= std::numeric_limits<BEntrySize>::max());

    mType = EVariantType::matrix;
    mError = EVariantError::none;
    mRowCount = static_cast<BEntrySize>(Source.size());

    for (const auto& List : Source)
    {
        assert(List.size() <= std::numeric_limits<BEntrySize>::max());
        mColumnCount = std::max(mColumnCount, static_cast<BEntrySize>(List.size()));
    }

    ConstructEntries();
    ResizeEntries(mRowCount * mColumnCount);

    CNumber* Entry = mEntries.get();
    for (auto itList = Source.begin(); itList != Source.end(); ++itList, Entry += mColumnCount)
    {
        std::ranges::copy(*itList, Entry);
    }
}

CMatrix& CMatrix::operator = (const CMatrix& Another)
{
    if (Another.mRowCount > 0)
    {
        assert(Another.mColumnCount > 0);
        assert(Another.mEntries.get());

        ResizeEntries(Another.mRowCount * Another.mColumnCount);
        std::ranges::copy(std::span(Another.mEntries.get(), mCapacity), mEntries.get());
    }

    mType = Another.mType;
    mError = Another.mError;
    mRowCount = Another.mRowCount;
    mColumnCount = Another.mColumnCount;
    return *this;
}

CMatrix& CMatrix::operator = (CMatrix&& Another) noexcept
{
    if (Another.mRowCount > 0)
    {
        assert(Another.mColumnCount > 0);
        assert(Another.mEntries.get());

        mCapacity = Another.mCapacity;
        mEntries = std::move(Another.mEntries);
    }

    mType = Another.mType;
    mError = Another.mError;
    mRowCount = Another.mRowCount;
    mColumnCount = Another.mColumnCount;
    return *this;
}

CMatrix& CMatrix::operator = (std::initializer_list< std::initializer_list<CNumber> > Source)
{
    BEntrySize RowCount = static_cast<BEntrySize>(Source.size());
    BEntrySize ColumnCount = 0;
    for (const auto& List : Source)
    {
        ColumnCount = std::max(ColumnCount, static_cast<BEntrySize>(List.size()));
    }
    if (ColumnCount <= 0)
    {
        RowCount = 0;
    }

    int NewEntryCount = RowCount * ColumnCount;
    if (NewEntryCount > 0)
    {
        ResizeEntries(NewEntryCount);

        CNumber* Entry = mEntries.get();
        for (auto itList = Source.begin(); itList != Source.end(); ++itList, Entry += ColumnCount)
        {
            std::ranges::copy(*itList, Entry);
        }
    }

    mRowCount = RowCount;
    mColumnCount = ColumnCount;
    mError = EVariantError::none;
    return *this;
}

CNumber& CMatrix::operator [] (int Row, int Column)
{
    assert(Row >= 0 && Row < mRowCount);
    assert(Column >= 0 && Column < mColumnCount);

    int Index = mColumnCount * Row + Column;
    return mEntries[Index];
}

CNumber CMatrix::operator [] (int Row, int Column) const
{
    assert(Row >= 0 && Row < mRowCount);
    assert(Column >= 0 && Column < mColumnCount);

    int Index = mColumnCount * Row + Column;
    return mEntries[Index];
}

CMatrix& CMatrix::operator += (const CMatrix& Another)
{
    if (mRowCount != Another.GetRowCount() || mColumnCount != Another.GetColumnCount())
    {
        SetInvalid(EVariantError::invalidSize);
        return *this;
    }

    auto itAnother = Another.begin();
    for (auto it = begin(), itEnd = end(); it != itEnd; ++it, ++itAnother)
    {
        *it += *itAnother;
    }

    return *this;
}

CMatrix& CMatrix::operator -= (const CMatrix& Another)
{
    if (mRowCount != Another.GetRowCount() || mColumnCount != Another.GetColumnCount())
    {
        SetInvalid(EVariantError::invalidSize);
        return *this;
    }

    auto itAnother = Another.begin();
    for (auto it = begin(), itEnd = end(); it != itEnd; ++it, ++itAnother)
    {
        *it -= *itAnother;
    }

    return *this;
}

CMatrix& CMatrix::operator *= (const CMatrix& Another)
{
    if (!IsMatrix() || !Another.IsMatrix() || mColumnCount != Another.GetRowCount())
    {
        SetInvalid(EVariantError::invalidSize);
        return *this;
    }

    CMatrix Product(0, mRowCount, Another.GetColumnCount());
    for (int Row = 0; Row < Product.mRowCount; Row++)
    {
        for (int Col = 0; Col < Product.mColumnCount; Col++)
        {
            for (int i = 0; i < mColumnCount; i++)
            {
                Product[Row, Col] += (*this)[Row, i] * Another[i, Col]; // TODO: Rework for performance.
            }
        }
    }

    *this = std::move(Product);
    return *this;
}

CMatrix& CMatrix::operator *= (CNumber Value)
{
    for (auto it = begin(), itEnd = end(); it != itEnd; ++it)
    {
        *it *= Value;
    }

    return *this;
}

CMatrix CMatrix::operator + (const CMatrix& Another) const
{
    CMatrix Clone = *this;
    Clone += Another;
    return Clone;
}

CMatrix CMatrix::operator - (const CMatrix& Another) const
{
    CMatrix Clone = *this;
    Clone -= Another;
    return Clone;
}

CMatrix CMatrix::operator * (const CMatrix& Another) const
{
    CMatrix Clone = *this;
    Clone *= Another;
    return Clone;
}

CMatrix CMatrix::operator * (CNumber Value) const
{
    CMatrix Clone = *this;
    Clone *= Value;
    return Clone;
}

CNumber* CMatrix::begin()
{
    if (IsEmpty())
    {
        return nullptr;
    }
    else
    {
        return mEntries.get();
    }
}

CNumber* CMatrix::end()
{
    if (IsEmpty())    
    {
        return nullptr;
    }
    else
    {
        assert(mRowCount * mColumnCount > 0);
        assert(mRowCount * mColumnCount <= mCapacity);

        return mEntries.get() + mRowCount * mColumnCount;
    }
}

bool CMatrix::IsEqual(const CMatrix& Another) const
{
    if (mType != Another.mType)
    {
        return false;
    }
    if (HasError() || Another.HasError())
    {
        return false;
    }

    if (mRowCount != Another.mRowCount)
    {
        return false;
    }
    if (mColumnCount != Another.mColumnCount)
    {
        return false;
    }

    if (mRowCount == 0)
    {
        return true;
    }

    auto itAnother = Another.begin();
    for (auto it = begin(), itEnd = end(); it != itEnd; ++it, ++itAnother)
    {
        if (!it->IsEqual(*itAnother))
        {
            return false;
        }
    }

    return true;
}

bool CMatrix::IsAlmostEqual(const CMatrix& Another, CNumber Tolerance) const
{
    if (mType != Another.mType)
    {
        return false;
    }
    if (HasError() || Another.HasError())
    {
        return false;
    }

    if (mRowCount != Another.mRowCount)
    {
        return false;
    }
    if (mColumnCount != Another.mColumnCount)
    {
        return false;
    }

    if (mRowCount == 0)
    {
        return true;
    }

    auto itAnother = Another.begin();
    for (auto it = begin(), itEnd = end(); it != itEnd; ++it, ++itAnother)
    {
        if (!it->IsAlmostEqual(*itAnother, Tolerance))
        {
            return false;
        }
    }

    return true;
}

CNumber CMatrix::GetManhattanDistance(const CMatrix& Another) const
{
    if (mRowCount != Another.mRowCount)
    {
        return CNumber::Null;
    }

    if (mColumnCount != Another.mColumnCount)
    {
        return CNumber::Null;
    }

    if (HasError() || Another.HasError())
    {
        return CNumber::Null;
    }

    if (mRowCount == 0)
    {
        return 0;
    }

    CNumber Dist = 0;
    auto itAnother = Another.begin();
    for (auto it = begin(), itEnd = end(); it != itEnd; ++it)
    {
        Dist += std::abs(*it - *itAnother);
    }

    return Dist;
}

std::string CMatrix::ToString() const
{
    if (HasError())
    {
        return "[!]";
    }

    std::string Return = "[";

    auto it = begin();
    for (int Row = 0; Row < mRowCount; Row++)
    {
        if (Row > 0)
        {
            Return += "|";
        }

        for (int Col = 0; Col < mColumnCount; Col++)
        {
            if (Col > 0)
            {
                Return += ' ';
            }

            Return += it->ToString();
            ++it;
        }
    }

    Return += "]";
    return Return;
}

void CMatrix::SetInvalid(EVariantError Error)
{
    mError = Error;
    mRowCount = 0;
    mColumnCount = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
