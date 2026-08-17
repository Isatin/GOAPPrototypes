// Copyright 2026 Isaac Hsu

#include <cassert>

#include "Matrix.h"
#include "Set.h"
#include "Variant.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
bool EVariantType::IsCompatible(EFactType FactType) const
{
    switch (mValue)
    {
    case EVariantType::unset:   return false;
    case EVariantType::scalar:  return FactType.IsScalar();
    case EVariantType::matrix:  return FactType == EFactType::matrix;
    case EVariantType::set:     return FactType == EFactType::set;
    }

    assert(!"Invalid variant type");
    return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CVariantBase::ConstructEntries()
{
    mCapacity = 0;
    new (&mEntries) std::unique_ptr<CNumber[]>();
}

void CVariantBase::DestructEntries()
{
    mCapacity = 0;
    mEntries.~unique_ptr<CNumber[]>();
}

void CVariantBase::ResizeEntries(int NewSize)
{
    assert(NewSize >= 0 && NewSize <= std::numeric_limits<BEntrySize>::max());

    if (NewSize > mCapacity)
    {
        mCapacity = NewSize;
        mEntries.reset(new CNumber[NewSize]);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const CVariant CVariant::Unset;

CVariant::~CVariant()
{
    switch (mType)
    {
    case EVariantType::matrix:
        reinterpret_cast<CMatrix&>(*this).~CMatrix();
        break;

    case EVariantType::set:
        reinterpret_cast<CSet&>(*this).~CSet();
        break;
    }
}

CVariant::CVariant()
{
    mType = EVariantType::unset;
    mError = EVariantError::none;
    mRowCount = 0;
    mColumnCount = 0;
    mCapacity = 0;
}

CVariant::CVariant(const CVariant& Another)
{
    switch (Another.mType)
    {
    default:
        mType = EVariantType::unset;
        mError = EVariantError::none;
        break;

    case EVariantType::scalar:
        mType = EVariantType::scalar;
        mError = Another.mError;
        mScalar = Another.mScalar;
        break;
    
    case EVariantType::matrix:
        new (this) CMatrix(reinterpret_cast<const CMatrix&>(Another));
        break;

    case EVariantType::set:
        new (this) CSet(reinterpret_cast<const CSet&>(Another));
        break;
    }
}

CVariant::CVariant(CVariant&& Another) noexcept
{
    switch (Another.mType)
    {
    default:
        mType = EVariantType::unset;
        mError = EVariantError::none;
        break;

    case EVariantType::scalar:
        mType = EVariantType::scalar;
        mError = Another.mError;
        mScalar = Another.mScalar;
        break;

    case EVariantType::matrix:
        new (this) CMatrix(std::move(reinterpret_cast<CMatrix&>(Another)));
        break;

    case EVariantType::set:
        new (this) CSet(std::move(reinterpret_cast<CSet&>(Another)));
        break;
    }
}

CVariant::CVariant(CNumber Value)
{
    mType = EVariantType::scalar;
    mError = EVariantError::none;
    mRowCount = 0;
    mColumnCount = 0;
    mCapacity = 0;
    mScalar = Value;
}

CVariant::CVariant(const CMatrix& Source)
{
    new (this) CMatrix(Source);
}

CVariant::CVariant(CMatrix&& Source)
{
    new (this) CMatrix(std::move(Source));
}

CVariant::CVariant(const CSet& Source)
{
    new (this) CSet(Source);
}

CVariant::CVariant(CSet&& Source)
{
    new (this) CSet(std::move(Source));
}

CVariant& CVariant::operator = (const CVariant& Another)
{
    assert(mType == EVariantType::unset || mType == Another.mType); // Only unset variants can change their types.

    if (mType == EVariantType::unset)
    {
        switch (Another.mType)
        {
        case EVariantType::matrix:  new (this) CMatrix(); break;
        case EVariantType::set:     new (this) CSet(); break;
        default:                    mType = Another.mType; break;
        }
    }

    switch (Another.mType)
    {
    case EVariantType::scalar:
        mError = Another.mError;
        mScalar = Another.mScalar;
        break;

    case EVariantType::matrix:
        reinterpret_cast<CMatrix&>(*this) = (reinterpret_cast<const CMatrix&>(Another));
        break;

    case EVariantType::set:
        reinterpret_cast<CSet&>(*this) = (reinterpret_cast<const CSet&>(Another));
        break;
    }

    return *this;
}

CVariant& CVariant::operator = (CVariant&& Another) noexcept
{
    assert(mType == EVariantType::unset || mType == Another.mType); // Only unset variants can change their types.

    if (mType == EVariantType::unset)
    {
        switch (Another.mType)
        {
        case EVariantType::matrix:  new (this) CMatrix(); break;
        case EVariantType::set:     new (this) CSet(); break;
        default:                    mType = Another.mType; break;
        }
    }

    switch (Another.mType)
    {
    case EVariantType::scalar:
        mError = Another.mError;
        mScalar = Another.mScalar;
        break;

    case EVariantType::matrix:
        reinterpret_cast<CMatrix&>(*this) = std::move(reinterpret_cast<CMatrix&>(Another));
        break;

    case EVariantType::set:
        reinterpret_cast<CSet&>(*this) = std::move(reinterpret_cast<CSet&>(Another));
        break;
    }

    return *this;
}

CVariant& CVariant::operator = (CNumber Value)
{
    assert(mType == EVariantType::unset || mType == EVariantType::scalar);

    mType = EVariantType::scalar;
    mError = EVariantError::none;
    mScalar = Value;
    return *this;
}

CVariant& CVariant::operator = (const CMatrix& Source)
{
    assert(mType == EVariantType::unset || mType == EVariantType::matrix);

    reinterpret_cast<CMatrix&>(*this) = Source;
    return *this;
}

CVariant& CVariant::operator = (CMatrix&& Source)
{
    assert(mType == EVariantType::unset || mType == EVariantType::matrix);

    reinterpret_cast<CMatrix&>(*this) = std::move(Source);
    return *this;
}

CVariant& CVariant::operator = (const CSet& Source)
{
    assert(mType == EVariantType::unset || mType == EVariantType::set);

    reinterpret_cast<CSet&>(*this) = Source;
    return *this;
}

CVariant& CVariant::operator = (CSet&& Source)
{
    assert(mType == EVariantType::unset || mType == EVariantType::set);

    reinterpret_cast<CSet&>(*this) = std::move(Source);
    return *this;
}

CNumber CVariant::GetScalar() const
{
    if (IsScalar())
    {
        return mScalar;
    }
    else
    {
        return CNumber::Null;
    }
}

CNumber CVariant::GrabScalar() const
{
    assert(IsScalar());

    return mScalar;
}

CNumber& CVariant::GrabScalar()
{
    assert(IsScalar());

    return mScalar;
}

const CMatrix& CVariant::GetMatrix() const
{
    if (IsMatrix())
    {
        return reinterpret_cast<const CMatrix&>(*this);
    }
    else
    {
        return CMatrix::InvalidType;
    }
}

const CMatrix& CVariant::GrabMatrix() const
{
    assert(IsMatrix());
    
    return reinterpret_cast<const CMatrix&>(*this);
}

CMatrix& CVariant::GrabMatrix() 
{
    assert(IsMatrix());

    return reinterpret_cast<CMatrix&>(*this);
}

const CSet& CVariant::GetSet() const
{
    if (IsSet())
    {
        return reinterpret_cast<const CSet&>(*this);
    }
    else
    {
        return CSet::InvalidType;
    }
}

const CSet& CVariant::GrabSet() const
{
    assert(IsSet());

    return reinterpret_cast<const CSet&>(*this);
}

CSet& CVariant::GrabSet()
{
    assert(IsSet());

    return reinterpret_cast<CSet&>(*this);
}

bool CVariant::IsEqual(const CVariant& Another) const
{
    if (mType != Another.mType)
    {
        return false;
    }
    if (mError != Another.mError)
    {
        return false;
    }

    switch (mType)
    {
    case EVariantType::unset:   return true;
    case EVariantType::scalar:  return mScalar.IsEqual(Another.mScalar);
    case EVariantType::matrix:  return GrabMatrix().IsEqual(Another.GrabMatrix());
    case EVariantType::set:     return GrabSet().IsEqual(Another.GrabSet());
    }

    assert(!"Invalid variant type");
    return false;
}

bool CVariant::IsAlmostEqual(const CVariant& Another, CNumber Tolerance) const
{
    if (mType != Another.mType)
    {
        return false;
    }
    if (mError != Another.mError)
    {
        return false;
    }

    switch (mType)
    {
    case EVariantType::unset:   return true;
    case EVariantType::scalar:  return mScalar.IsAlmostEqual(Another.mScalar, Tolerance);
    case EVariantType::matrix:  return GrabMatrix().IsAlmostEqual(Another.GrabMatrix(), Tolerance);
    case EVariantType::set:     return GrabSet().IsAlmostEqual(Another.GrabSet(), Tolerance);
    }

    assert(!"Invalid variant type");
    return false;
}

std::string CVariant::ToString() const
{
    switch (mType)
    {
#ifdef USE_UNICODE_SYMBOLS
    case EVariantType::unset:   return "⍰";
#else
    case EVariantType::unset:   return "?";
#endif
    case EVariantType::scalar:  return mScalar.ToString();
    case EVariantType::matrix:  return GrabMatrix().ToString();
    case EVariantType::set:     return GrabSet().ToString();
    }

    assert(!"Invalid variant type");
    return "";
}
///////////////////////////////////////////////////////////////////////////////////////////////////