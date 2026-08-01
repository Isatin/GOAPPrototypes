// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows that a customizable GOAP system can be used to find the shortest route for a task. 
// Suppose that there are three ammo boxes and three guns in a level, and an agent at the origin tries 
// to secure a gun and some ammo to shoot its target. The first gun is the closest gun to the origin,
// and the second ammo box is the closest ammo to the origin. However, taking the third gun and ammo 
// box results in the shortest path. 
// 
// A locally optimal algorithm may go to the closest gun or ammo then go to the closest remaining item,
// which fails to find the shortest route. By contrast, GOAP can look ahead, take more possibilities 
// into consideration, and find the best solution.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"
#include "GOAP/GOAP.h"
#include "Vector.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
static float gDistanceCost = 0.1f;
///////////////////////////////////////////////////////////////////////////////////////////////////
class CStateWithPosition : public CState
{
    using Super = CState;
public:
    using Super::Super;

    const SVector& GetPosition() const { return mPosition; }
    void SetPosition(const SVector& Value) { mPosition = Value; }
    void SetPosition(float X, float Y) { mPosition.Set(X, Y); }

protected:
    std::unique_ptr<CState> Clone() const override { return std::make_unique<CStateWithPosition>(*this);}

    float GetExtraHeuristicCost(const CState& Another) const override
    {
        const CStateWithPosition* State1 = dynamic_cast<const CStateWithPosition*>(this);
        if (!State1)
            return 0.f;

        const CStateWithPosition* State2 = dynamic_cast<const CStateWithPosition*>(&Another);
        if (!State2)
            return 0.f;

        const SVector& Pos1 = State1->GetPosition();
        if (!Pos1.IsValid())
            return 0.f;

        const SVector& Pos2 = State2->GetPosition();
        if (!Pos2.IsValid())
            return 0.f;

        return (Pos1 - Pos2).Length() * gDistanceCost;
    }

    std::string ToString() const override
    {
        std::string Return = Super::ToString();
        if (!mPosition.IsValid())
        {
            return Return;
        }

        Return += ", Pos=";
        Return += mPosition.ToString();
        return Return;
    }

private:
    SVector mPosition{NAN};
};
///////////////////////////////////////////////////////////////////////////////////////////////////
class CActionWithPosition : public CAction
{
    using Super = CAction;
public:
    using Super::Super;

    const SVector& GetPosition() const { return mPosition; }
    void SetPosition(const SVector& Value) { mPosition = Value; }
    void SetPosition(float X, float Y) { mPosition.Set(X, Y); }

protected:
    void Affect(CState& State) const override
    {
        if (CStateWithPosition* PosState = dynamic_cast<CStateWithPosition*>(&State))
        {
            PosState->SetPosition(mPosition);
        }
    }

    float GetCustomCost(const CState& CurrentState, const CState& NextState) const override
    {
        const CStateWithPosition* CurrPosState = dynamic_cast<const CStateWithPosition*>(&CurrentState);
        if (!CurrPosState)
            return 1.f;

        const CStateWithPosition* NextPosState = dynamic_cast<const CStateWithPosition*>(&NextState);
        if (!NextPosState)
            return 1.f;

        const SVector& CurrPos = CurrPosState->GetPosition();
        if (!CurrPos.IsValid())
            return 1.f;

        const SVector& NextPos = NextPosState->GetPosition();
        if (!NextPos.IsValid())
            return 1.f; 

        return 1.f + (CurrPos - NextPos).Length() * gDistanceCost;
    }

    std::string ToString() const override
    {
        std::string Return = Super::ToString();
        if (!mPosition.IsValid())
        {
            return Return;
        }

        Return += " Pos=";
        Return += mPosition.ToString();
        return Return;
    }

private:
    SVector mPosition{NAN};
};
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    SVector AmmoPositions[]= { {6.f, 3.f}, {4.f, -3.f}, {-6.f, 0.f} };
    SVector GunPositions[] = { {3.f, 3.f}, {7.f, -3.f}, {-5.f, 0.f} };

    CStateWithPosition StartingState;
    StartingState.SetProperty("GunReady", false);
    StartingState.SetProperty("HasAmmo", false);
    StartingState.SetProperty("HasGun", false);
    StartingState.SetPosition(0.f, 0.f);

    CStateWithPosition GoalState;
    GoalState.SetProperty("GunReady", true);

    std::vector<const CAction*> Actions;

    const int AmmoCount = sizeof(AmmoPositions) / sizeof(AmmoPositions[0]);
    std::vector<CActionWithPosition> GetAmmoActions(AmmoCount, CActionWithPosition("GA"));
    for (int i = 0; i < AmmoCount; i++)
    {
        CActionWithPosition& GetAmmo = GetAmmoActions[i];
        GetAmmo.SetName(GetAmmo.GetName() + static_cast<char>('1' + i));
        GetAmmo.SetPrecondition("HasAmmo", false);
        GetAmmo.SetEffect("HasAmmo", true);
        GetAmmo.SetPosition(AmmoPositions[i]);
        Actions.push_back(&GetAmmo);
    }

    const int GunCount = sizeof(GunPositions) / sizeof(GunPositions[0]);
    std::vector<CActionWithPosition> GetGunActions(GunCount, CActionWithPosition("GG"));
    for (int i = 0; i < GunCount; i++)
    {
        CActionWithPosition& GetGun = GetGunActions[i];
        GetGun.SetName(GetGun.GetName() + static_cast<char>('1' + i));
        GetGun.SetPrecondition("HasGun", false);
        GetGun.SetEffect("HasGun", true);
        GetGun.SetPosition(GunPositions[i]);
        Actions.push_back(&GetGun);
    }

    CAction Reload("R");
    Reload.SetPrecondition("HasAmmo", true);
    Reload.SetPrecondition("HasGun", true);
    Reload.SetEffect("GunReady", true);
    Actions.push_back(&Reload);

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
