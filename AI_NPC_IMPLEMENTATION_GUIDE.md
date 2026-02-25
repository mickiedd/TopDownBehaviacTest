# AI NPC Implementation Guide for TopDownBehaviacTest

## 🎯 Overview
This guide will help you create an AI NPC character controlled by BehaviacPlugin in your TopDownBehaviacTest project.

## ✅ Files Created

### C++ Files (Already created)
- `BehaviacAINPC.h` - AI Character header
- `BehaviacAINPC.cpp` - AI Character implementation
- `Content/AI/BT_SimpleNPC.xml` - Example behavior tree XML

## 📋 Step-by-Step Implementation

### Step 1: Compile C++ Code
1. **Close Unreal Editor** if it's open
2. **Regenerate project files:**
   - Right-click `TopDownBehaviacTest.uproject`
   - Select "Generate Xcode Project" (Mac) or "Generate Visual Studio Project" (Windows)
3. **Open the project in Unreal Editor**
   - It will detect new code and ask to rebuild
   - Click **"Yes"** to rebuild modules
4. Wait for compilation to finish

### Step 2: Create Behavior Tree Data Asset
1. In **Content Browser**, create folder: `Content/AI`
2. Right-click in Content/AI → **Miscellaneous** → **Data Asset**
3. Select **`UBehaviacBehaviorTree`**
4. Name it: **`DA_NPCBehaviorTree`**
5. Double-click to open it
6. Set properties:
   - Tree Name: "SimpleNPC"
   - Version: 1

### Step 3: Import Behavior Tree XML (Optional)
If you want to use XML-based behavior trees:
1. Copy `BT_SimpleNPC.xml` to your content folder
2. In the Data Asset, use "LoadFromXML" to import it

OR create nodes programmatically (recommended for testing).

### Step 4: Create AI Character Blueprint
1. In Content Browser, right-click in Content/AI
2. **Blueprint Class** → Search for **`BehaviacAINPC`**
3. Select your C++ class
4. Name it: **`BP_AINPC`**
5. Open the Blueprint

### Step 5: Configure Blueprint
In **BP_AINPC** Blueprint:

#### Class Defaults:
- **Behavior Tree**: Select `DA_NPCBehaviorTree`
- **Detection Radius**: `1000.0`
- **Walk Speed**: `200.0`
- **Run Speed**: `400.0`

#### Mesh Setup:
1. Select **Mesh (inherited)** component
2. Set **Skeletal Mesh**: Use any character mesh you have
   - Try: `/Game/ThirdPerson/Characters/Mannequins/Meshes/SKM_Quinn` (if available)
   - Or use a simple capsule for testing

#### AI Controller:
1. **Class Settings** → **Pawn** section
2. **AI Controller Class**: `AIController` (built-in)

### Step 6: Place NPC in Level
1. Open **TopDownMap** level
2. Drag **`BP_AINPC`** from Content Browser into the level
3. Position it away from the player character
4. In Details panel, verify:
   - Auto Possess AI: **Placed in World or Spawned**
   - Behavior Tree is assigned

### Step 7: Test Basic Functionality

#### Quick Test (Before Behavior Tree):
1. **Play** the level (PIE)
2. Check **Output Log** for:
   ```
   BehaviacAINPC: Loaded behavior tree for BP_AINPC_C_0
   ```
3. The NPC should stand idle initially

#### Full Test (With Behavior Tree):
1. The NPC should:
   - **Patrol** between waypoints when player is far
   - **Detect** player when within 1000 units
   - **Chase** player when detected
   - **Return to patrol** when player moves away

## 🎨 Visual Debugging

### Add Debug Visualization:
In BP_AINPC Event Graph, add:

```
Event Tick
├─ Draw Debug Sphere
│  ├─ Center: Get Actor Location
│  ├─ Radius: Detection Radius (1000)
│  ├─ Color: Green (if no target) / Red (if has target)
│  └─ Duration: 0.0 (always visible)
```

### Enable AI Debugging:
- Press **'** (apostrophe) key in PIE to show AI debug info
- Shows navigation paths and AI state

## 🔧 Behavior Tree Logic

The simple behavior tree implements:

### Main Loop:
1. **Selector** (tries children until one succeeds)
   - **Sequence**: Chase Player
     - Precondition: Check if player detected
     - Action: MoveToTarget (player)
   - **Sequence**: Patrol (fallback)
     - Action: Patrol waypoints
     - Wait: 2 seconds at each point
2. **Action**: FindPlayer (always run to update detection)

### AI Methods (Called by Behavior Tree):
- `FindPlayer()` - Detects player within radius
- `MoveToTarget(AActor*)` - Moves to specific target
- `Patrol()` - Cycles through patrol points
- `Idle()` - Stops movement
- `IsPlayerInRange()` - Checks distance to player

## 🚀 Advanced Customization

### Add More Behaviors:
1. **Attack behavior** - when close to player
2. **Flee behavior** - when low health
3. **Investigate** - check last known position

### Extend C++ Class:
Add to `BehaviacAINPC.h`:
```cpp
UFUNCTION(BlueprintCallable, Category = "AI|Actions")
bool AttackTarget();

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
float AttackRange;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
float AttackDamage;
```

### Create Multiple NPC Types:
- Create child Blueprints of `BP_AINPC`
- Assign different behavior trees
- Adjust speeds, detection ranges

## 📊 Property System (Blackboard)

The NPC uses Behaviac's property system like UE's Blackboard:

### Set Properties from C++:
```cpp
SetBehaviacProperty(TEXT("Health"), TEXT("100"));
SetBehaviacProperty(TEXT("State"), TEXT("Patrol"));
SetBehaviacProperty(TEXT("TargetLocation"), TEXT("X=100,Y=200,Z=0"));
```

### Read in Behavior Tree:
Use conditions to check property values in XML/nodes.

## 🐛 Troubleshooting

### NPC doesn't move:
- ✅ Check Nav Mesh exists (press **P** in viewport)
- ✅ Verify AI Controller is assigned
- ✅ Check behavior tree is loaded (output log)

### Behavior tree not loading:
- ✅ Ensure BehaviacPlugin is enabled
- ✅ Check DA_NPCBehaviorTree is assigned in Blueprint
- ✅ Verify C++ code compiled successfully

### NPC moves erratically:
- ✅ Add Nav Mesh Bounds Volume to level
- ✅ Adjust movement speeds
- ✅ Check patrol point positions

## 📚 Next Steps

1. ✅ Add attack/interaction behavior
2. ✅ Create multiple NPC types with different trees
3. ✅ Add animations (idle, walk, run, attack)
4. ✅ Implement health and combat system
5. ✅ Add visual effects (damage, particles)
6. ✅ Create NPC spawner system

## 💡 Tips

- Use **Blueprint** for quick iteration of behavior
- Use **C++** for performance-critical code
- Test behavior trees incrementally (start simple!)
- Use debug visualization during development
- Log frequently to understand behavior flow

---

**Need help?** Check:
- BehaviacPlugin documentation: http://www.behaviac.com/
- Unreal AI documentation: https://docs.unrealengine.com/en-US/AI/
