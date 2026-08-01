// Copyright 2024 Isaac Hsu

#pragma once

#include <string>

#include "Effect.h"
#include "State.h"


namespace ArithGOAP
{
    struct SNumericFactRange;
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CAction // Named action with preconditions and effects
    {
    public:
        CAction(const std::string& Name, const CFactDefinition& Definition);
        virtual ~CAction() {}

        // Return a description of this action for debugging.
        virtual std::string ToString() const;
        // Is this action feasible in the forward search?
        virtual bool CheckPrecondition(const CState& State) const;
        // Reverse this action from a given state and return the previous state, if feasible.
        virtual std::unique_ptr<CState> CheckPostcondition(const CState& Postcondition) const;
        // Overridable function invoked on the state of each search node for customization. Do NOT modify the properties. 
        virtual void Affect(CState& State) const {}
        // Customizable cost of this action
        virtual float GetCustomCost(const CState& CurrentState, const CState& NextState) const { return 0.f; }

        const std::string& GetName() const { return mName; }
        void SetName(const std::string& Value) { mName = Value; }

        float GetBaseCost() const { return mBaseCost; }
        void SetBaseCost(float Value) { mBaseCost = Value; }

        const CFactDefinition& GetDefinition() const { return mPrecondition.GetDefinition(); }

        CState& GetPrecondition() { return mPrecondition; }
        const CState& GetPrecondition() const { return mPrecondition; }
        bool SetPrecondition(const CFact& Fact, CNumber Value)  { return mPrecondition.SetProperty(Fact, Value); }
        bool SetPrecondition(const SFactEquation& Equation)     { return mPrecondition.SetProperty(Equation); }
        bool SetPrecondition(const SNumericFactRange& Range)    { return mPrecondition.SetProperty(Range); }

        const CEffect& GetEffect() const { return mEffect; }
        bool SetEffect(const CFact& Fact, CNumber Value)        { return mEffect.SetTransform(Fact, Value); }
        bool SetEffect(const SFactOperation& Operation)         { return mEffect.SetTransform(Operation); }

        // Total cost of this action
        float GetCost(const CState& CurrentState, const CState& NextState) const { return mBaseCost + GetCustomCost(CurrentState, NextState); }

    private:
        std::string mName;
        float mBaseCost = 1.f;
        CState mPrecondition;
        CEffect mEffect;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
