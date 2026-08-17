// Copyright 2025 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is an algebraic version of the namesake in GOAP.sln.
// You may reference the namesake for more information.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ExampleUtility/ExampleUtility.h"
#include "Vector.h"


using namespace AlgebGOAP;
///////////////////////////////////////////////////////////////////////////////////////////////////
static float gDistanceCost = 0.1f;
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename TBase>
class CAddOnPosition : public TBase
{
    using Super = TBase;
public:
    using Super::Super;

    const SVector& GetPosition() const { return mPosition; }
    void SetPosition(const SVector& Value) { mPosition = Value; }
    void SetPosition(float X, float Y) { mPosition.Set(X, Y); }

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

protected:
    std::unique_ptr<TBase> Clone() const override { return std::make_unique<CAddOnPosition>(*this); }

protected:
    SVector mPosition{NAN};
};
///////////////////////////////////////////////////////////////////////////////////////////////////
class CConditionWithPosition : public CAddOnPosition<CCondition>
{
    using Super = CAddOnPosition<CCondition>;
public:
    using Super::Super;

    std::unique_ptr<CCondition> Clone() const override { return std::make_unique<CConditionWithPosition>(*this); }
};
///////////////////////////////////////////////////////////////////////////////////////////////////
class CStateWithPosition : public CAddOnPosition<CState>
{
    using Super = CAddOnPosition<CState>;
public:
    using Super::Super;

    std::unique_ptr<CState> Clone() const override { return std::make_unique<CStateWithPosition>(*this); }

    float GetExtraHeuristicCost(const CCondition& Another) const
    {
        const CStateWithPosition* State = dynamic_cast<const CStateWithPosition*>(this);
        if (!State)
            return 0.f;

        const CAddOnPosition<CCondition>* Condition = dynamic_cast<const CAddOnPosition<CCondition>*>(&Another);
        if (!Condition)
            return 0.f;

        const SVector& StatePos = State->GetPosition();
        if (!StatePos.IsValid())
            return 0.f;

        const SVector& ConditionPos = Condition->GetPosition();
        if (!ConditionPos.IsValid())
            return 0.f;

        return (StatePos - ConditionPos).Length() * gDistanceCost;
    }
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
    std::unique_ptr<CAction> Clone() const override { return std::make_unique<CActionWithPosition>(*this); }

    template <typename T>
    void Affect(T& Base) const
    {
        if (CAddOnPosition<T>* AddOn = dynamic_cast<CAddOnPosition<T>*>(&Base))
        {
            AddOn->SetPosition(mPosition);
        }
    }

    void Affect(CState& State) const override { Affect<CState>(State); }
    void Affect(CCondition& Condition) const override { Affect<CCondition>(Condition); }

    template <typename T>
    float GetCustomCost(const T& Current, const T& Next) const
    {
        const CAddOnPosition<T>* CurrAddOn = dynamic_cast<const CAddOnPosition<T>*>(&Current);
        if (!CurrAddOn)
            return 1.f;

        const CAddOnPosition<T>* NextAddOn = dynamic_cast<const CAddOnPosition<T>*>(&Next);
        if (!NextAddOn)
            return 1.f;

        const SVector& CurrPos = CurrAddOn->GetPosition();
        if (!CurrPos.IsValid())
            return 1.f;

        const SVector& NextPos = NextAddOn->GetPosition();
        if (!NextPos.IsValid())
            return 1.f;

        return 1.f + (CurrPos - NextPos).Length() * gDistanceCost;
    }

    float GetCustomCost(const CState& Current, const CState& Next) const override { return GetCustomCost<CState>(Current, Next); }
    float GetCustomCost(const CCondition& Current, const CCondition& Next) const override { return GetCustomCost<CCondition>(Current, Next); }

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

protected:
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
    auto& AmmoInc   = *Definition.DefineNumber("AmmoInc");
    auto& GunInc    = *Definition.DefineNumber("GunInc");

    CStateWithPosition StartingState(Definition);
    StartingState.SetProperty(GunReady, false);
    StartingState.SetProperty(Ammo, 0);
    StartingState.SetProperty(Gun, 0);
    StartingState.SetProperty(AmmoInc, 10);
    StartingState.SetProperty(GunInc, 1); 
    StartingState.SetPosition(0.f, 0.f);

    CConditionWithPosition GoalState(Definition);
    GoalState.SetConstraint(GunReady == true);

    std::vector<const CAction*> Actions;

    const int AmmoCount = sizeof(AmmoPositions) / sizeof(AmmoPositions[0]);
    std::vector<CActionWithPosition> GetAmmoActions(AmmoCount, CActionWithPosition("GA", Definition));
    for (int i = 0; i < AmmoCount; i++)
    {
        CActionWithPosition& GetAmmo = GetAmmoActions[i];
        GetAmmo.SetName(GetAmmo.GetName() + static_cast<char>('1' + i));
        GetAmmo.SetPrecondition(Ammo == 0);
        GetAmmo.AddEffect(Ammo += AmmoInc);
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
        GetGun.AddEffect(Gun += GunInc);
        GetGun.SetPosition(GunPositions[i]);
        Actions.push_back(&GetGun);
    }

    CAction Reload("R", Definition);
    Reload.SetPrecondition(Ammo > 0 && Gun > 0);
    Reload.AddEffect(GunReady = true);
    Actions.push_back(&Reload);

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
