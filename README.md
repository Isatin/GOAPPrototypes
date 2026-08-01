GOAP Prototypes
=========
This is my research project for extending a game AI technique called Goal-Oriented Action Planning (GOAP) in C++.  
I shared it online because it might be useful or inspiring to developers interested in GOAP.  
However, my work focuses on only the planning algorithms and does not address goal selection and plan execution.  

There are two independent groups of projects:
- `GOAP` supports Boolean and enumeration world properties.
- `ArithGOAP` adds support for numeric world properties and arithmetic operations.

Three planner classes are implemented across the two groups:
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

Although written in Visual Studio for Windows, the code should compile and run in any C++20 IDE with a proper setup, as these projects are simply console programs depending only on the STL.

## Manual

The library code for `GOAP` and `ArithGOAP` is placed in their own namespaces.  
In the code, world states are simply called states and world properties are called facts.  
Goals and preconditions are also expressed as states.

### `GOAP`

#### World State
The world state type, `CState`, is used for various GOAP components, such as the starting and goal states, and the preconditions and effects of actions. It can be set up by calling its member function `SetProperty`, as shown in the following example.
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
Alternatively, call `RunGOAPs` to invoke multiple planners.  

### `ArithGOAP`

In the namespace `ArithGOAP`, you can use the following C++ operators on facts to set up world states and actions:
| types of world property | world states/preconditoins | effects                     |
|-------------------------|----------------------------|-----------------------------|
| Boolean                 | `==`                       | `=`, `!`                    |
| enumeration             | `==`                       | `=`                         |
| number                  | `==`, `<=`, `>=`           | `=`, `+=`, `-=`, `*=`, `/=` |
#### World Property  
The first thing to do is to define world properties with a `CStateDefinition` instance like the code below.
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
