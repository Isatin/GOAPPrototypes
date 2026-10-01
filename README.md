GOAP Prototypes
=========
This is my experimental C++ project for extending a game AI technique called Goal-Oriented Action Planning (GOAP).  
I shared it online because it might be useful or inspiring to developers interested in GOAP.  
However, my work focuses on only the planning algorithms and does not address goal selection and plan execution.  

There are four independent groups of code projects:
- `GOAP` supports Boolean and enumeration world properties.
- `ArithGOAP` adds support for numeric world properties and arithmetic operations.
- `AlgebGOAP` enables the use of algebraic expressions for effects and logical formulas composed of relational expressions for preconditions and goals.
- `ExtendedGOAP` extends the domain from scalars to matrices and numerical sets, providing corresponding matrix and set operations.

Three planner classes are implemented across the four groups:
- `CForwardPlanner`
- `CRegressionPlanner`
- `CAdvRegressionPlanner`: an improved version of `CRegressionPlanner`

In addtion, a modified version of the original regressive GOAP, named `CBackwardPlanner`, is available in `GOAP` for reference.

## Quick Start
1. Download and install Visual Studio 2022 Community Edition.
2. Open a solution file (.sln) with Visual Studio 2022.
3. In Solution Explorer, right click an example project and select "Set as Startup Project" from the context menu.
4. Press F5 to compile and run the selected project.
5. The results are shown in the console window.

Although written in Visual Studio for Windows, the code should compile and run in any C++23 IDE with a proper setup, as these projects are simply console programs depending only on the STL.

## Manual

The library code for `GOAP`, `ArithGOAP`, `AlgebGOAP`, and `ExtendedGOAP` is placed in their respective folders and namespaces. 
In the code, world states are simply called states and world properties are also called facts.  

### `GOAP`

#### World State
The world state type, `CState`, is used for various GOAP components, such as the starting and goal states, and the preconditions and effects of actions in `GOAP`. It can be set up by calling its member function `SetProperty`, as shown in the following example.
```
enum EPlace {bedroom, kitchen, workshop};

CState StartingState;
StartingState.SetProperty("Where", bedroom);
StartingState.SetProperty("Hungry", true);

CState GoalState;
GoalState.SetProperty("Hungry", false);
```
#### Action
The action type, `CAction`, can be set up by calling its member functions `SetPrecondition` and `SetEffect`, as shown below.
```
CAction Eat("Eat");
Eat.SetPrecondition("Where", kitchen);
Eat.SetEffect("Hungry", false);
```
#### Planning
Pack the actions into a `std::vector` and call `ForwardSearch`/`BackwardSearch`/`RegressiveSearch`/`AdvRegressiveSearch`.  
Alternatively, call `RunGOAPs` to execute multiple planners.  

### `ArithGOAP`

In the namespace `ArithGOAP`, you can use the following C++ operators on facts to define world states and actions:
| types of world properties | world states/preconditoins | effects                     |
|---------------------------|----------------------------|-----------------------------|
| Boolean                   | `==`                       | `=`, `!`                    |
| enumeration               | `==`                       | `=`                         |
| number                    | `==`, `<=`, `>=`           | `=`, `+=`, `-=`, `*=`, `/=` |
#### World Property  
The first step is to define facts using a `CFactDefinition` instance like the code below.
```
CFactDefinition Definition;
auto& Where     = *Definition.DefineEnumeration("Where");
auto& HasHammer = *Definition.DefineBoolean("HasHammer");
auto& Wood      = *Definition.DefineNumber("Wood");
auto& Table     = *Definition.DefineNumber("Table");
```
#### World State
You can configure start states by calling `SetProperty`—as seen in `GOAP`—and set up goal states using the `==` operator on facts, or the `<=` and `>=` operators on numeric facts, as follows:  
```
CState StartingState(Definition);
StartingState.SetProperty(Where, bedroom);
StartingState.SetProperty(HasHammer, false);
StartingState.SetProperty(Wood, 10);
StartingState.SetProperty(Table, 0);

CState GoalState(Definition);
GoalState.SetProperty(Where == kitchen);
GoalState.SetProperty(Table >= 1);
```
#### Action
You can create actions with `CAction`, set up preconditions like goal states, and specify effects using the operator `=` on facts, or the `+=`, `-=`, `*=`, and `/=` operators on numeric facts, as shown in the following snippet.  
```
CAction Craft("Craft", Definition);
Craft.SetPrecondition(Where == workshop);
Craft.SetPrecondition(HasHammer == true);
Craft.SetPrecondition(Wood >= 3);
Craft.SetEffect(Wood -= 3);
Craft.SetEffect(Table += 1);
```
#### Planning  
Pack the actions into a `std::vector` and call `ForwardSearch`/`RegressiveSearch`/`AdvRegressiveSearch`  
or use `RunGOAPs` for multiple planners.  

### `AlgebGOAP`

The setup of world properties and starting states is similar to that of `ArithGOAP`. 
The main difference is in the formulation of effects, preconditions, and goals. 
World properties can be combined with operators and math functions listed below to compose algebraic expressions for effects.
Also, two expressions can be connected using the following comparators to form relational expressions for preconditions and goals.
Furthermore, logical connectives such as `&&`, `||`, and `!` can link relational expressions into a logical formula for preconditions and goals as well.
| types of world properties | comparators                      | operators/functions                          |
|---------------------------|----------------------------------|----------------------------------------------|
| Boolean                   | `==`, `!=`                       | `!`, `&&`, `\|\|`                            |
| enumeration               | `==`, `!=`                       | N/A                                          |
| number                    | `==`, `!=`, `<`, `<=`, `>`, `>=` | `+`, `-`, `*`, `/`, `%`, `abs`, `min`, `max` |
#### World Property  
Likewise, we first need to define facts using a `CFactDefinition` instance.
```
CFactDefinition Definition;
auto& MaxHP     = *Definition.DefineNumber("MaxHP");
auto& HP        = *Definition.DefineNumber("HP");
auto& SP        = *Definition.DefineNumber("SP");
auto& HealSP    = *Definition.DefineNumber("HealSP");
auto& Base      = *Definition.DefineNumber("Base");
auto& Coef      = *Definition.DefineNumber("Coef");
auto& Bonus     = *Definition.DefineNumber("Bonus");
```
#### World State
Starting states are still constructed with the `CState` class and set up in the same way.
```
CState StartingState(Definition);
StartingState.SetProperty(MaxHP, 100);
StartingState.SetProperty(HP, 40);
StartingState.SetProperty(SP, 100);
StartingState.SetProperty(HealSP, 40);
StartingState.SetProperty(Base, 30);
StartingState.SetProperty(Coef, 1.5);
StartingState.SetProperty(Bonus, 20);
```
#### Goal
However, goals are created using the `CCondition` class instead and configured with a relational expression or a logical formula.
```
CCondition GoalCondition(Definition);
GoalCondition.SetConstraint(HP >= MaxHP);
```
#### Action
Preconditions are set up like goals, and effects are defined by assigning algebraic expressions to facts.  
An action can have only one precondtion formula but multiple effects. Note that the order of effects matters.  
```
CAction Heal("Heal", Definition);
Heal.SetPrecondition(HP < MaxHP && SP >= HealSP);
Heal.AddEffect(HP += Base * Coef + Bonus);
Heal.AddEffect(HP = min(HP, MaxHP));
Heal.AddEffect(SP -= HealSP);
```
#### Planning  
Planners are invoked in the same way, by calling `ForwardSearch`/`RegressiveSearch`/`AdvRegressiveSearch` or `RunGOAPs`.

### `ExtendedGOAP`

`ExtendedGOAP` is an extended version of `AlgebGOAP` with additional support for matrices, numerical sets, and their operations. 
Since most set operators are not available in C++, they are replaced by those shown in the following table. 
| types of world properties       | comparators                                   | operators                |
|---------------------------------|-----------------------------------------------|--------------------------|
| matrix                          | `==`, `!=`                                    | `+`, `-`, `*`            |
| set                             | `==`, `!=`, `<`, `<=`, `>`, `>=`, `<<`, `>>`  | `&`, `\|`, `-`, `^`, `~` |
| **corresponding set operators** | `==`, `≠` , `⊂`, `⊆` , `⊃`, `⊇` , `∈` , `∋` | `∩`, `∪`, `∖`, `△`, `ᶜ`  |
