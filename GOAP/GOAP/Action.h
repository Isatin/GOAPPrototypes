// Copyright 2024 Isaac Hsu

#pragma once

#include "State.h"


namespace GOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CAction // Action with preconditions and effects
    {
    public:
        CAction(const std::string& Name);
        virtual ~CAction() {}

        const std::string& GetName() const { return mName; }
        void SetName(const std::string& Value) { mName = Value; }

        const CState& GetPrecondition() const { return mPrecondition; }
        void SetPrecondition(const std::string& Key, PFact Value) { mPrecondition.SetFact(Key, Value); }

        const CState& GetEffect() const { return mEffect; }
        void SetEffect(const std::string& Key, PFact Value) { mEffect.SetFact(Key, Value); }

        // Return a description of preconditions and effects
        virtual std::string ToString() const;
        // For forward search to check feasibility
        virtual bool CheckPrecondition(const CState& State) const; 
        // For regressive search to check feasibility
        virtual bool CheckPostcondition(const CState& State) const; 
        // Customizable cost of the action
        virtual float GetCost(const CState& CurrentState, const CState& NextState) const { return 1.f; }
        // Overridable function invoked on the state of each search node for customization
        virtual void Affect(CState& State) const {} 

    private:
        std::string mName;
        CState mPrecondition;
        CState mEffect;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
}