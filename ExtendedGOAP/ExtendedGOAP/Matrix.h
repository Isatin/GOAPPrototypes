// Copyright 2026 Isaac Hsu

#pragma once

#include <initializer_list>

#include "Variant.h"


namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CMatrix : public CVariantBase // Numeric matrix
    {
    public:
        static const CMatrix Empty;
        static const CMatrix InvalidType;
        static const CMatrix InvalidSize;

    public:        
        ~CMatrix();
        CMatrix(EVariantError Error = EVariantError::none);
        CMatrix(const CMatrix& Another);
        CMatrix(CMatrix&& Another) noexcept;
        CMatrix(CNumber Value, int RowCount, int ColumnCount);
        CMatrix(std::initializer_list< std::initializer_list<CNumber> > Source);

        CMatrix& operator = (const CMatrix& Another);
        CMatrix& operator = (CMatrix&& Another) noexcept;
        CMatrix& operator = (std::initializer_list< std::initializer_list<CNumber> > Source);
        bool operator == (const CMatrix& Another) const { return IsAlmostEqual(Another); }
        CNumber& operator [] (int Row, int Column);
        CNumber operator [] (int Row, int Column) const;

        CMatrix& operator += (const CMatrix& Another);
        CMatrix& operator -= (const CMatrix& Another);
        CMatrix& operator *= (const CMatrix& Another);
        CMatrix& operator *= (CNumber Value);

        [[nodiscard]] CMatrix operator + (const CMatrix& Another) const;
        [[nodiscard]] CMatrix operator - (const CMatrix& Another) const;
        [[nodiscard]] CMatrix operator * (const CMatrix& Another) const;
        [[nodiscard]] CMatrix operator * (CNumber Value) const;

        CNumber* begin();
        CNumber* end();
        const CNumber* begin() const    { return const_cast<CMatrix*>(this)->begin(); }
        const CNumber* end() const      { return const_cast<CMatrix*>(this)->end(); }

        bool IsEmpty() const        { return mRowCount <= 0; }
        bool IsSingleton() const    { return mRowCount == 1 && mColumnCount == 1; }
        bool IsEqual(const CMatrix& Another) const;
        bool IsAlmostEqual(const CMatrix& Another, CNumber Tolerance = CNumber::GetDefaultTolerance()) const;
        int GetRowCount() const     { return mRowCount; }
        int GetColumnCount() const  { return mColumnCount; }
        int GetCapacity() const     { return mCapacity; }
        CNumber GetManhattanDistance(const CMatrix& Another) const;
        std::string ToString() const;

        void SetInvalid(EVariantError Error);
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
