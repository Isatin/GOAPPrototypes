// Copyright 2025 Isaac Hsu

#pragma once

#include <memory>

#include "BitVector.h"
#include "Condition.h"
#include "Effect.h"


namespace AlgebGOAP
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class CAction // Named action with preconditions and effects
    {
    public:
        CAction(const std::string& Name, const CFactDefinition& Definition, float BaseCost = 1.f);
        virtual ~CAction() {}

        // Overridable clone function. Subclasses should override to return their own type.
        virtual std::unique_ptr<CAction> Clone() const { return std::make_unique<CAction>(*this); }
        // Return a description of this action for debugging.
        virtual std::string ToString() const;
        // Is this action feasible in the forward search?
        virtual bool CheckPrecondition(const CState& State) const;
        // Overridable function invoked on the state of each search node for customization. Do NOT modify the properties. 
        virtual void Affect(CState& State) const {}
        // Overridable function invoked on the condition of each search node for customization. Do NOT modify the constraints. 
        virtual void Affect(CCondition& Condition) const {}
        // Customizable cost of this action
        virtual float GetCustomCost(const CState& CurrentState, const CState& NextState) const { return 0.f; }
        virtual float GetCustomCost(const CCondition& CurrentCondition, const CCondition& NextCondition) const { return 0.f; }
        
        // Total cost of this action
        float GetCost(const CState& CurrentState, const CState& NextState) const;
        float GetCost(const CCondition& CurrentCondition, const CCondition& NextCondition) const;

        const std::string& GetName() const { return mName; }
        void SetName(const std::string& Value) { mName = Value; }

        float GetBaseCost() const { return mBaseCost; }
        void SetBaseCost(float Value);

        const CFactDefinition& GetDefinition() const { return mPrecondition.GetDefinition(); }
        CBitVector GetUsedFactBits() const { return mPrecondition.GetUsedFactBits() | mEffect.GetUsedFactBits(); }

        CCondition& GetPrecondition() { return mPrecondition; }
        const CCondition& GetPrecondition() const { return mPrecondition; }
        template<typename... TArgs>
        void SetPrecondition(TArgs&&... Args) { mPrecondition.SetConstraint(std::forward<TArgs>(Args)...); }

        const CEffect& GetEffect() const { return mEffect; }
        template<typename... TArgs>
        auto AddEffect(TArgs&&... Args) { return mEffect.AddTransform(std::forward<TArgs>(Args)...); }
        void ClearEffect() { mEffect.ClearTransforms(); }

        template <typename T>
        void ReplaceFacts(T&& Replacements);
        void Rearrange();

    private:
        std::string mName;
        float mBaseCost = 1.f;
        CCondition mPrecondition;
        CEffect mEffect;
    };
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    void CAction::ReplaceFacts(T&& Replacements)
    {
        mPrecondition.ReplaceFacts(std::forward<T>(Replacements));
        mEffect.ReplaceFacts(std::forward<T>(Replacements));
    }
}
