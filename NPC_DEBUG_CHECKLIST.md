# BP_AINPC Debug Checklist

## Common Reasons NPC Won't Move:

### 1. ❌ **No Nav Mesh** (Most Common!)
**Solution:**
- In level viewport, add **Nav Mesh Bounds Volume**
- Scale it to cover the entire floor
- Press **P** key to visualize (should show green overlay)

### 2. ❌ **No AI Controller Assigned**
**Check in BP_AINPC:**
- Open Class Defaults
- Under **Pawn** section:
  - **AI Controller Class** = `AIController` (built-in)
  - **Auto Possess AI** = `Placed in World or Spawned`

### 3. ❌ **Behavior Tree Not Set**
**In BP_AINPC Class Defaults:**
- **Behavior Tree** property = Should point to a Data Asset
- If empty, create one:
  1. Content/AI → Right-click → Data Asset → `UBehaviacBehaviorTree`
  2. Name: `DA_NPCBehaviorTree`
  3. Assign it to the NPC

### 4. ❌ **No Skeletal Mesh / Collision**
**Check in BP_AINPC:**
- **Mesh (Inherited)** component has a mesh assigned
- **Capsule Component** has collision enabled

### 5. ❌ **Movement Component Issues**
**In Character Movement component:**
- **Max Walk Speed** > 0 (should be 200)
- **Can Walk** = True
- **Movement Mode** = Walking

## 🛠️ **Quick Test:**

### Test 1: Manual Movement
Add to BP_AINPC Event Graph:
```
Event Tick
├─ Add Movement Input
│  ├─ World Direction: (1, 0, 0)
│  └─ Scale Value: 1.0
```
If this makes it move → AI/Nav Mesh issue
If this doesn't work → Movement component issue

### Test 2: AI Controller Check
Add to BeginPlay:
```
Event BeginPlay
├─ Print String
│  └─ In String: Get Controller → Get Display Name
```
Should print "AIController" (not "PlayerController")

### Test 3: Behavior Tree Execution
Check Output Log for:
- "BehaviacAINPC: Patrolling to point X"
- "BehaviacAINPC: Moving to target"
If missing → Behavior tree not executing

## 📝 **Enable Debug Logging:**

In Editor Console (` key):
```
log LogTemp Verbose
log LogNavigation Verbose
log LogAINavigation Verbose
```

Play again and check Output Log.

## 🎯 **Most Likely Fix:**

**Add Nav Mesh Bounds Volume:**
1. Place Modes → Volumes → **Nav Mesh Bounds Volume**
2. Drag into level
3. Scale to cover walkable area
4. Press **P** to see green nav mesh
5. If no green → Nav Mesh not building
   - Check Project Settings → Navigation System → Runtime Generation = Dynamic

**Then test again!**
