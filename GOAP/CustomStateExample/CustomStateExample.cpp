// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example shows that GOAP can be used to find the shortest route for something
// and that customizable GOAP systems could be useful.
// 
// Suppose that there are three ammo boxes and three guns in the virtual world.
// The first gun is the closest gun to the origin, and the second ammo box is the closest ammo to the origin,
// while the third gun and ammo are the cloest route.
// A local optimal algorithm may go get the closest gun or ammo then go get the closest ammo or gun,
// which would fail to find the shortest route in this case.
// By contrast, GOAP can look ahead and take more possibilities into consideration.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <iostream>
#include <sstream>

#include "ExampleUtility/ExampleUtility.h"
#include "GOAP/GOAP.h"
#include "Vector.h"


using namespace GOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
static float gDistanceCost = 0.1f;
///////////////////////////////////////////////////////////////////////////////////////////////////
class CStateWithPosition : public CState
{
public:
    using Super = CState;
    using Super::Super;

    const SVector& GetPosition() const { return mPosition; }
    void SetPosition(const SVector& Value) { mPosition = Value; }
    void SetPosition(float X, float Y) { mPosition.Set(X, Y); }

protected:
    std::unique_ptr<CState> Clone() const override { return std::make_unique<CStateWithPosition>(*this);}

    std::string ToString() const override 
    {
        std::stringstream Stream;
        Stream << std::fixed << std::setprecision(2);
        Stream << Super::ToString();
        Stream << ", (" << mPosition.X << "," << mPosition.Y << ")";
        return Stream.str();
    }

    float GetExtraHeuristicCost(const CState& Another) const override
    {
        const CStateWithPosition* CurrentPosState = dynamic_cast<const CStateWithPosition*>(this);
        if (!CurrentPosState)
            return 0.f;

        const CStateWithPosition* GoalPosState = dynamic_cast<const CStateWithPosition*>(&Another);
        if (!GoalPosState)
            return 0.f;

        const SVector& CurrentPos = CurrentPosState->GetPosition();
        if (!CurrentPos.IsValid())
            return 0.f;

        const SVector& GoalPos = GoalPosState->GetPosition();
        if (!GoalPos.IsValid())
            return 0.f;

        return (CurrentPos - GoalPos).Length() * gDistanceCost;
    }

private:
    SVector mPosition{ NAN };
};
///////////////////////////////////////////////////////////////////////////////////////////////////
class CActionWithPosition : public CAction
{
public:
    using Super = CAction;
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

    float GetCost(const CState& CurrentState, const CState& NextState) const override
    {
        const CStateWithPosition* CurrentPosState = dynamic_cast<const CStateWithPosition*>(&CurrentState);
        if (!CurrentPosState)
            return 1.f;

        const CStateWithPosition* NextPosState = dynamic_cast<const CStateWithPosition*>(&NextState);
        if (!NextPosState)
            return 1.f;

        const SVector& CurrentPos = CurrentPosState->GetPosition();
        if (!CurrentPos.IsValid())
            return 1.f;

        const SVector& NextPos = NextPosState->GetPosition();
        if (!NextPos.IsValid())
            return 1.f; 

        return 1.f + (CurrentPos - NextPos).Length() * gDistanceCost;
    }

private:
    SVector mPosition{ 0.f };
};
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    SVector AmmoPositions[]= { {6.f, 3.f}, {4.f, -3.f}, {-6.f, 0.f} };
    SVector GunPositions[] = { {3.f, 3.f}, {7.f, -3.f}, {-5.f, 0.f} };

    CStateWithPosition StartingState;
    StartingState.SetFact("GunReady", false);
    StartingState.SetFact("HasAmmo", false);
    StartingState.SetFact("HasGun", false);
    StartingState.SetPosition(0.f, 0.f);

    CStateWithPosition GoalState;
    GoalState.SetFact("GunReady", true);

    std::vector<const CAction*> Actions;

    const int AmmoCount = sizeof(AmmoPositions) / sizeof(AmmoPositions[0]);
    std::vector<CActionWithPosition> GetAmmoActions(AmmoCount, CActionWithPosition("GA"));
    for (int i = 0; i < AmmoCount; i++)
    {
        CActionWithPosition& Action = GetAmmoActions[i];
        Action.SetName(Action.GetName() + static_cast<char>('1' + i));
        Action.SetPrecondition("HasAmmo", false);
        Action.SetEffect("HasAmmo", true);
        Action.SetPosition(AmmoPositions[i]);
        Actions.push_back(&Action);
    }

    const int GunCount = sizeof(GunPositions) / sizeof(GunPositions[0]);
    std::vector<CActionWithPosition> GetGunActions(GunCount, CActionWithPosition("GG"));
    for (int i = 0; i < GunCount; i++)
    {
        CActionWithPosition& Action = GetGunActions[i];
        Action.SetName(Action.GetName() + static_cast<char>('1' + i));
        Action.SetPrecondition("HasGun", false);
        Action.SetEffect("HasGun", true);
        Action.SetPosition(GunPositions[i]);
        Actions.push_back(&Action);
    }

    CAction Reload("R");
    Reload.SetPrecondition("HasAmmo", true);
    Reload.SetPrecondition("HasGun", true);
    Reload.SetEffect("GunReady", true);
    Actions.push_back(&Reload);

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
