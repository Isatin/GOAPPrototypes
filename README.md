GOAP prototypes
=========
This is my personal research program of GOAP in C++.  
I share it online because it may contain useful information for others interested in GOAP.

There are two independent sets of projects:
- `GOAP` supports world properties of Boolean and enumerations.
- `ArithGOAP` additionally supports world properties of floating-point numbers with arithmetic operations.

Three planning algorithms are implemented in both `GOAP` and `ArithGOAP`:
- forward search
- regressive search
- regressive search with lookup tables

However, my work focuses on only the planning algorithms and does not contain goal selection and plan execution.

## Quickstart
1. Download and install Visual Studio 2022 Community Edition
2. Open the solution files (GOAP.sln or ArithGOAP.sln) with Visual Studio 2022
3. In Solution Explorer, right click one of the example projects and left click popup menu item "Set as Startup Project"
4. Press F5 to compile and run the selected project
5. Check the results in console window

Although the code is written in Visual Studio on Windows, it should be able to get compiled and run in other C++20 IDEs with proper setup since the projects are just console programs and they only depend on STL.

## Manual

The library code of `GOAP` and `ArithGOAP` is put in their respective namespaces.  
World states are just called states and world properties are called facts in the code.  
Goals are simply described as goal states.

### `GOAP`

#### World State
The base class of world state is `CState` and it's used for everything such as the starting and goal states, preconditions and effects of actions.  
It can be set up by calling its member function `SetFact` as the following example:
```
enum EPlace {bedroom, kitchen, studio};

CState StartingState;
StartingState.SetFact("Where", bedroom);
StartingState.SetFact("Hungry", true);

CState GoalState;
GoalState.SetFact("Hungry", false);
```
#### Action
The base class of action is `CAction` and it can be set up by calling its member function `SetPrecondition` & `SetEffect` as follows:
```
CAction Eat("Eat");
Eat.SetPrecondition("Where", kitchen);
Eat.SetEffect("Hungry", false);
```
#### Planning
You need to pack the actions in a `std::vector` and call `ForwardSearch`/`RegressiveSearch`/`LUTRegressiveSearch`  
or call `RunGOAPs` for testing all three.

### `ArithGOAP`

In namespace `ArithGOAP`, you can use the following C++ operators on facts to set up world states and actions:
| types of world property | world states/preconditoins | effects                     |
|-------------------------|----------------------------|-----------------------------|
| Boolean                 | `==`                       | `=`, `!`                    |
| enumeration             | `==`                       | `=`                         |
| floating-point number   | `==`, `<=`, `>=`           | `=`, `+=`, `-=`, `*=`, `/=` |
#### World Property  
The first thing you need to do is to define your world properties with a `CStateDefinition` instance like:
```
CStateDefinition Definition;
auto& WhereFact     = *Definition.DefineEnumeration("Where");
auto& HasHammerFact = *Definition.DefineBoolean("HasHammer");
auto& WoodFact      = *Definition.DefineNumber("Wood");
auto& TableFact     = *Definition.DefineNumber("Table");
```
#### World State
The base class of world state is `CState` and you can set it up with operator `==` on all facts or `<=`, `>=` on numeric facts as the following:
```
CState StartingState(Definition);
StartingState.SetFact(WhereFact == bedroom);
StartingState.SetFact(HasHammerFact == false);
StartingState.SetFact(WoodFact == 10);
StartingState.SetFact(TableFact == 0);

CState GoalState(Definition);
GoalState.SetFact(TableFact >= 1);
```
#### Action
The base class of action is `CAction` and you can set up preconditions like world states and set up effects with operator `=` on all facts, or `+=`, `-=`, `*=`, `/=` on numeric facts like:
```
CAction Craft("Craft", Definition);
Craft.SetPrecondition(WhereFact == studio);
Craft.SetPrecondition(HasHammerFact == true);
Craft.SetPrecondition(WoodFact >= 3);
Craft.SetEffect(WoodFact -= 3);
Craft.SetEffect(TableFact += 1);
```
#### Planning  
You need to pack the actions in a `std::vector` and call `ForwardSearch`/`RegressiveSearch`/`LUTRegressiveSearch`  
or call `RunGOAPs` for testing all three.
