// Copyright 2026 Isaac Hsu

#pragma once

#include <cstdint>
#include <memory>

#include "FactType.h"
#include "Number.h"


namespace ExtendedGOAP
{
    class CMatrix;
    class CSet;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class EVariantType // Mock enum of variant types
    {
    public:
        enum class Type : unsigned char
        {
            unset,
            scalar,
            matrix,
            set,
        };

        static constexpr Type unset     = Type::unset;
        static constexpr Type scalar    = Type::scalar;
        static constexpr Type matrix    = Type::matrix;
        static constexpr Type set       = Type::set;

    public:
        EVariantType() = default;
        EVariantType(Type Value) : mValue(Value) {}

        operator Type() const { return mValue; }
        bool IsCompatible(EFactType FactType) const;

    private:
        Type mValue = unset;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    enum class EVariantError : unsigned char // Enum of variant error types
    {
        none,
        invalidType,
        invalidSize,
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CVariantBase // Base class for a compact variant that can be a scalar, a matrix, or a set of numbers
    {
    public:
        using BEntrySize = int16_t;

        EVariantType GetVariantType() const { return mType; }
        EVariantError GetError() const { return mError; }
        bool IsUnset() const    { return mType == EVariantType::unset; }
        bool IsScalar() const   { return mType == EVariantType::scalar; }
        bool IsMatrix() const   { return mType == EVariantType::matrix; }
        bool IsSet() const      { return mType == EVariantType::set; }
        bool HasError() const   { return mError != EVariantError::none; }

    protected:
        CVariantBase() {}
        ~CVariantBase() {}

        void ConstructEntries();
        void DestructEntries();
        void ResizeEntries(int NewSize);

    protected:
        EVariantType mType;
        EVariantError mError;
        union
        {
            BEntrySize mEntryCount;
            BEntrySize mRowCount;
        };
        union
        {
            BEntrySize mColumnCount;
            BEntrySize mComplement;
        };
        BEntrySize mCapacity;
        union
        {
            CNumber mScalar;
            std::unique_ptr<CNumber[]> mEntries;
        };
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Compact variant for storing a world property value
    // NOTE: To keep it simple, changing to other variant types is not allowed, as it is unnecessary for this project.
    class CVariant : public CVariantBase
    {
    public:
        static const CVariant Unset;

    public:
        ~CVariant();
        CVariant();
        CVariant(const CVariant& Another);
        CVariant(CVariant&& Another) noexcept;
        CVariant(CNumber Value);
        CVariant(const CMatrix& Source);
        CVariant(CMatrix&& Source);
        CVariant(const CSet& Source);
        CVariant(CSet&& Source);

        CVariant& operator = (const CVariant& Another);
        CVariant& operator = (CVariant&& Another) noexcept;
        CVariant& operator = (CNumber Value);
        CVariant& operator = (const CMatrix& Source);
        CVariant& operator = (CMatrix&& Source);
        CVariant& operator = (const CSet& Source);
        CVariant& operator = (CSet&& Source);
        bool operator == (const CVariant& Another) const { return IsAlmostEqual(Another); }

        CNumber GetScalar() const;
        CNumber GrabScalar() const;
        CNumber& GrabScalar();
        const CMatrix& GetMatrix() const;
        const CMatrix& GrabMatrix() const;
        CMatrix& GrabMatrix();
        const CSet& GetSet() const;
        const CSet& GrabSet() const;
        CSet& GrabSet();

        bool IsEqual(const CVariant& Another) const;
        bool IsAlmostEqual(const CVariant& Another, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        std::string ToString() const;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}