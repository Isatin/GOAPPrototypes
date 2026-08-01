// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is an arithmetic version of the namesake in GOAP.sln.
// You may reference the namesake for more information.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"
#include "Vector.h"


using namespace ArithGOAP;
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
    std::unique_ptr<CState> Clone() const override { return std::make_unique<CStateWithPosition>(*this); }

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

    CFactDefinition Definition;
    auto& GunReady  = *Definition.DefineBoolean("GunReady");
    auto& Ammo      = *Definition.DefineNumber("Ammo");
    auto& Gun       = *Definition.DefineNumber("Gun");

    CStateWithPosition StartingState(Definition);
    StartingState.SetProperty(GunReady, false);
    StartingState.SetProperty(Ammo, 0);
    StartingState.SetProperty(Gun, 0);
    StartingState.SetPosition(0.f, 0.f);

    CStateWithPosition GoalState(Definition);
    GoalState.SetProperty(GunReady, true);

    std::vector<const CAction*> Actions;

    const int AmmoCount = sizeof(AmmoPositions) / sizeof(AmmoPositions[0]);
    std::vector<CActionWithPosition> GetAmmoActions(AmmoCount, CActionWithPosition("GA", Definition));
    for (int i = 0; i < AmmoCount; i++)
    {
        CActionWithPosition& GetAmmo = GetAmmoActions[i];
        GetAmmo.SetName(GetAmmo.GetName() + static_cast<char>('1' + i));
        GetAmmo.SetPrecondition(Ammo == 0);
        GetAmmo.SetEffect(Ammo += 10);
        GetAmmo.SetPosition(AmmoPositions[i]);
        Actions.push_back(&GetAmmo);
    }

    const int GunCount = sizeof(GunPositions) / sizeof(GunPositions[0]);
    std::vector<CActionWithPosition> GetGunActions(GunCount, CActionWithPosition("GG", Definition));
    for (int i = 0; i < GunCount; i++)
    {
        CActionWithPosition& GetGun = GetGunActions[i];
        GetGun.SetName(GetGun.GetName() + static_cast<char>('1' + i));
        GetGun.SetPrecondition(Gun == 0);
        GetGun.SetEffect(Gun += 1);
        GetGun.SetPosition(GunPositions[i]);
        Actions.push_back(&GetGun);
    }

    CAction Reload("R", Definition);
    Reload.SetPrecondition(Ammo >= 1);
    Reload.SetPrecondition(Gun >= 1);
    Reload.SetEffect(GunReady, true);
    Actions.push_back(&Reload);

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
