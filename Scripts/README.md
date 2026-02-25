# Behaviac Behavior Tree Reimport Tools

Quick tools for reimporting behavior trees from XML during development.

## 🚀 Quick Start

### **Recommended Workflow:**

1. **Edit your XML file** (e.g., `Content/AI/BT_Test_Simple.xml`)

2. **Run the reimport script:**
   ```bash
   cd /Volumes/M2/Works/TopDownBehaviacTest
   ./Scripts/reimport_bt.sh Test_Simple
   ```

3. **Follow the on-screen instructions** - You'll see 3 options:
   - **Option 1**: Right-click asset → Reimport (if editor is open)
   - **Option 2**: Restart editor (auto-reimports on startup)
   - **Option 3**: Drag & drop XML into Content Browser

## 📋 Available Tools

### 1. `reimport_bt.sh` - Smart Reimport Helper

**What it does:**
- ✅ Validates XML file exists
- ✅ Shows XML size and modification time
- ✅ Deletes `.uasset` cache file
- ✅ Detects if editor is running
- ✅ Provides context-aware instructions

**Usage:**
```bash
./Scripts/reimport_bt.sh BT_Test_Simple
./Scripts/reimport_bt.sh Test_Simple  # BT_ prefix optional
```

### 2. `clear_bt_cache.sh` - Batch Cache Cleaner

**What it does:**
- Deletes `.uasset` cache files only
- Supports multiple files or all files
- Requires editor restart to take effect

**Usage:**
```bash
# Clear single file
./Scripts/clear_bt_cache.sh Test_Simple

# Clear multiple files
./Scripts/clear_bt_cache.sh Test_Simple SimpleNPC

# Clear ALL behavior trees
./Scripts/clear_bt_cache.sh
```

## 🎯 Three Ways to Reimport

### **Method 1: Editor Reimport Button** (Fastest if editor is open)

**Requirements:** Plugin rebuilt with reimport support (✅ Done!)

**Steps:**
1. Right-click the behavior tree asset in Content Browser
2. Select **"Reimport"**
3. Choose the XML file when prompted
4. ✅ Done instantly!

**Advantages:**
- ✅ No editor restart needed
- ✅ Fastest method
- ✅ See results immediately

### **Method 2: Delete Cache + Restart** (Most Reliable)

**Steps:**
1. Run `./Scripts/reimport_bt.sh BT_Name`
2. Restart the UE5 Editor
3. ✅ Asset auto-reimports from XML on startup

**Advantages:**
- ✅ Most reliable (guaranteed to work)
- ✅ No manual file selection needed
- ✅ Good for batch updates

### **Method 3: Drag & Drop** (For new files)

**Steps:**
1. Open UE5 Editor
2. Drag `BT_Name.xml` from Finder into Content Browser → AI folder
3. ✅ Automatically creates `.uasset`

**Advantages:**
- ✅ Intuitive
- ✅ Good for first-time import
- ✅ No command-line needed

## 🔧 Typical Workflow

### **Iterative Development:**

```bash
# 1. Edit XML
vim Content/AI/BT_Test_Simple.xml

# 2. Reimport
./Scripts/reimport_bt.sh Test_Simple

# 3. In UE5 Editor: Right-click → Reimport
#    (or restart editor)

# 4. Test
#    Play In Editor (PIE)
```

### **Batch Updates:**

```bash
# Clear all caches
./Scripts/clear_bt_cache.sh

# Restart editor - all BTs will reimport
```

## 📁 File Structure

```
TopDownBehaviacTest/
├── Content/AI/
│   ├── BT_Test_Simple.xml      ← Edit this (source)
│   └── BT_Test_Simple.uasset   ← Generated (delete to reimport)
│
└── Scripts/
    ├── reimport_bt.sh          ← Smart reimport helper
    ├── clear_bt_cache.sh       ← Batch cache cleaner
    ├── reimport_bt.py          ← Python helper (advanced)
    └── README.md               ← This file
```

## ⚠️ Important Notes

- **XML is the source of truth** - Always edit the `.xml` file, not properties in the editor
- **`.uasset` files are cached** - They won't update automatically when XML changes
- **Must reimport** after XML edits using one of the three methods above
- **Native Reimport button** only appears if plugin is rebuilt with latest code

## 🐛 Troubleshooting

**Q: XML changes not showing up?**  
A: Run `./Scripts/reimport_bt.sh YourBT` and follow the instructions

**Q: "Reimport" button missing from context menu?**  
A: Plugin needs to be rebuilt. Run:
```bash
cd /Volumes/M2/Works/TopDownBehaviacTest
./Scripts/clear_bt_cache.sh YourBT
# Then restart editor
```

**Q: Import failed / asset corrupted?**  
A: Delete `.uasset`, fix XML syntax errors, restart editor

**Q: Want to see what changed?**  
A: Check XML modification time:
```bash
stat -f "%Sm" Content/AI/BT_*.xml
```

## 📊 Status Check

**Check if assets are up-to-date:**
```bash
# List XML files with modification times
ls -lh Content/AI/BT_*.xml

# List .uasset files
ls -lh Content/AI/BT_*.uasset

# Compare timestamps
stat -f "%Sm %N" Content/AI/BT_Test_Simple.*
```

