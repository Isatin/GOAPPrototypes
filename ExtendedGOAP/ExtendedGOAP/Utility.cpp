// Copyright 2026 Isaac Hsu

#include "Utility.h"


using namespace ExtendedGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace ExtendedGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
#if defined(USE_UNICODE_SYMBOLS) && (defined(_WIN32) || defined(_WIN64))
    #include <windows.h>
    #undef min
    #undef max
    
    class CWindowsConfig
    {
    public:
        CWindowsConfig()
        {
            mOldCP = GetConsoleOutputCP();
            SetConsoleOutputCP(CP_UTF8);
        }

        ~CWindowsConfig()
        {
            SetConsoleOutputCP(mOldCP);
        }

    private:
        UINT mOldCP = 0;
    }
    gWindowsConfig;
#endif // USE_UNICODE_SYMBOLS
    ///////////////////////////////////////////////////////////////////////////////////////////////
    bool IsParenthetic(const std::string& String)
    {
        if (String.empty())
        {
            return false;
        }

        return String.front() == '(' && String.back() == ')';
    }

    std::string Parenthesize(const std::string& String)
    {
        std::string Return("()");
        Return.insert(1, String);
        return Return;
    }

    std::string& Parenthesize(std::string& String)
    {
        String.insert(0, "(");
        String.push_back(')');
        return String;
    }

    // It may look better with std::u8string, but the support is really limited in C++.
    // So, we are sticking with std::string for now.
    size_t CountCodepoints(const std::string& u8String)
    {
        size_t Count = 0;
        for (char8_t Byte : u8String)
        {
            if ((Byte & 0xC0) != 0x80)
            {
                Count++;
            }
        }

        return Count;
    }

    std::string Pad(const std::string& u8String, size_t Length, char Padding)
    {
        std::string u8StringToPad = u8String;        
        return Pad(u8StringToPad, Length, Padding);
    }

    std::string& Pad(std::string& u8String, size_t Length, char Padding)
    {
        size_t OldLength = CountCodepoints(u8String);
        if (OldLength < Length)
        {
            u8String.resize(Length + (u8String.size() - OldLength), Padding);
        }

        return u8String;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool SIndexedText::Compare(const SIndexedText& Left, const SIndexedText& Right)
{
    return Left.Text < Right.Text;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool SIndexedSizeAndText::Compare(const SIndexedSizeAndText& Left, const SIndexedSizeAndText& Right)
{ 
    if (Left.Size < Right.Size)
    {
        return true;
    }
    else  if (Left.Size > Right.Size)
    {
        return false;
    }
    else
    {
        return Left.Text < Right.Text;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////