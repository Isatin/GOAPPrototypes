// Copyright 2024 Isaac Hsu
///////////////////////////////////////////////////////////////////////////////////////////////////
// This example is a numeric version of the namesake in GOAP VS solution.
// You may reference the namesake for the more information.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <iostream>
#include <sstream>

#include "ExampleUtility/ExampleUtility.h"
#include "Vector.h"


using namespace ArithGOAP;
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
    std::unique_ptr<CState> Clone(std::vector<SInterval>&& Facts) const override { return std::make_unique<CStateWithPosition>(GetDefinition(), std::move(Facts)); }

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

    float GetCustomCost(const CState& CurrentState, const CState& NextState) const override
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

    CStateDefinition Definition;
    auto& GunReadyFact = *Definition.DefineBoolean("GunReady");
    auto& AmmoFact = *Definition.DefineNumber("Ammo");
    auto& GunFact = *Definition.DefineNumber("Gun");

    CStateWithPosition StartingState(Definition);
    StartingState.SetFact(GunReadyFact, false);
    StartingState.SetFact(AmmoFact, 0);
    StartingState.SetFact(GunFact, 0);
    StartingState.SetPosition(0.f, 0.f);

    CStateWithPosition GoalState(Definition);
    GoalState.SetFact(GunReadyFact, true);

    std::vector<const CAction*> Actions;

    const int AmmoCount = sizeof(AmmoPositions) / sizeof(AmmoPositions[0]);
    std::vector<CActionWithPosition> GetAmmoActions(AmmoCount, CActionWithPosition("GA", Definition));
    for (int i = 0; i < AmmoCount; i++)
    {
        CActionWithPosition& Action = GetAmmoActions[i];
        Action.SetName(Action.GetName() + static_cast<char>('1' + i));
        Action.SetPrecondition(AmmoFact == 0);
        Action.SetEffect(AmmoFact += 10);
        Action.SetPosition(AmmoPositions[i]);
        Actions.push_back(&Action);
    }

    const int GunCount = sizeof(GunPositions) / sizeof(GunPositions[0]);
    std::vector<CActionWithPosition> GetGunActions(GunCount, CActionWithPosition("GG", Definition));
    for (int i = 0; i < GunCount; i++)
    {
        CActionWithPosition& Action = GetGunActions[i];
        Action.SetName(Action.GetName() + static_cast<char>('1' + i));
        Action.SetPrecondition(GunFact == 0);
        Action.SetEffect(GunFact += 1);
        Action.SetPosition(GunPositions[i]);
        Actions.push_back(&Action);
    }

    CAction Reload("R", Definition);
    Reload.SetPrecondition(AmmoFact >= 1);
    Reload.SetPrecondition(GunFact >= 1);
    Reload.SetEffect(GunReadyFact, true);
    Actions.push_back(&Reload);

    RunGOAPs(StartingState, GoalState, Actions);
    return 0;
}
