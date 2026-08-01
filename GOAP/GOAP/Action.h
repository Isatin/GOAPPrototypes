// Copyright 2024 Isaac Hsu

#pragma once

#include "State.h"


namespace GOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CAction // Named action with preconditions and effects
    {
    public:
        CAction(const std::string& Name);
        virtual ~CAction() {}

        // Return a description of this action for debugging.
        virtual std::string ToString() const;
        // Is this action feasible in the forward search?
        virtual bool CheckPrecondition(const CState& State) const;
        // Is this action feasible in the regressive search?
        virtual bool CheckPostcondition(const CState& State) const;
        // Overridable function invoked on the state of each search node for customization. Do NOT modify the properties. 
        virtual void Affect(CState& State) const {}
        // Customizable action cost
        virtual float GetCustomCost(const CState& CurrentState, const CState& NextState) const { return 0.f; }

        const std::string& GetName() const { return mName; }
        void SetName(const std::string& Value) { mName = Value; }

        float GetBaseCost() const { return mBaseCost; }
        void SetBaseCost(float Value) { mBaseCost = Value; }

        const CState& GetPrecondition() const { return mPrecondition; }
        void SetPrecondition(const std::string& Name, BProperty Value) { mPrecondition.SetProperty(Name, Value); }

        const CState& GetEffect() const { return mEffect; }
        void SetEffect(const std::string& Name, BProperty Value) { mEffect.SetProperty(Name, Value); }

        // Total cost of this action
        float GetCost(const CState& CurrentState, const CState& NextState) const { return mBaseCost + GetCustomCost(CurrentState, NextState); }

    private:
        std::string mName;
        float mBaseCost = 1.f;
        CState mPrecondition;
        CState mEffect;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}