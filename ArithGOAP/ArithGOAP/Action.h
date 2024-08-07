// Copyright 2024 Isaac Hsu

#pragma once

#include <optional>
#include <string>

#include "Effect.h"
#include "State.h"


namespace ArithGOAP
{
    struct SNumericFactOperation;
    struct SNumericFactRange;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CAction // Action with preconditions and effects
    {
    public:
        CAction(const std::string& Name, const CStateDefinition& Def);
        virtual ~CAction() {}

        const std::string& GetName() const { return mName; }
        void SetName(const std::string& Value) { mName = Value; }

        float GetBaseCost() const { return mBaseCost; }
        void SetBaseCost(float Value) { mBaseCost = Value; }

        const CStateDefinition& GetDefinition() const { return mPrecondition.GetDefinition(); }

        CState& GetPrecondition() { return mPrecondition; }
        const CState& GetPrecondition() const { return mPrecondition; }
        bool SetPrecondition(const CFact& Fact, SNumber Value)  { return mPrecondition.SetFact(Fact, Value); }
        bool SetPrecondition(const SFactRange& Range)           { return mPrecondition.SetFact(Range); }
        bool SetPrecondition(const SNumericFactRange& Range)    { return mPrecondition.SetFact(Range); }

        const CEffect& GetEffect() const { return mEffect; }
        bool SetEffect(const CFact& Fact, SNumber Value)        { return mEffect.SetTransform(Fact, Value); }
        bool SetEffect(const SFactOperation& Operation)         { return mEffect.SetTransform(Operation); }
        bool SetEffect(const SBooleanFactOperation& Operation)  { return mEffect.SetTransform(Operation); }
        bool SetEffect(const SNumericFactOperation& Operation)  { return mEffect.SetTransform(Operation); }

        // Return a description of preconditions and effects
        virtual std::string ToString() const;
        // For forward search to check feasibility
        virtual bool CheckPrecondition(const CState& State) const;
        // For regressive search to check feasibility. If ok, return desired facts after the action is undone.
        virtual std::optional<std::vector<SInterval>> CheckPostcondition(const CState& State) const; 
        // Overridable function invoked on the state of each search node for customization. Modification to facts must NOT be made. 
        virtual void Affect(CState& State) const {}
        // Customizable cost of the action
        virtual float GetCustomCost(const CState& CurrentState, const CState& NextState) const { return 0.f; }
        // Total cost of the action
        float GetCost(const CState& CurrentState, const CState& NextState) const { return mBaseCost + GetCustomCost(CurrentState, NextState); }

    private:
        std::string mName;
        float mBaseCost = 1.f;
        CState mPrecondition;
        CEffect mEffect;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
