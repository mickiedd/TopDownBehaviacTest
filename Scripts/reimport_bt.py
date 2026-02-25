#!/usr/bin/env python3
"""
Behaviac Behavior Tree Reimport Tool
Properly imports XML files using UFactory system
"""

import unreal
import os

def import_behavior_tree_from_xml(xml_path, asset_name, asset_path="/Game/AI"):
    """
    Import a behavior tree from XML using the factory system
    
    Args:
        xml_path: Full path to XML file (e.g., "/path/to/BT_SimpleNPC.xml")
        asset_name: Asset name (e.g., "BT_SimpleNPC")
        asset_path: Destination path in content browser (e.g., "/Game/AI")
    
    Returns:
        bool: Success or failure
    """
    
    # Check if XML file exists
    if not os.path.exists(xml_path):
        unreal.log_error(f"❌ XML file not found: {xml_path}")
        return False
    
    file_size = os.path.getsize(xml_path)
    unreal.log_warning(f"📋 Found XML file: {xml_path} ({file_size} bytes)")
    
    # Delete existing asset first to force fresh import
    full_asset_path = f"{asset_path}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(full_asset_path):
        unreal.log_warning(f"🗑️  Deleting existing asset: {full_asset_path}")
        unreal.EditorAssetLibrary.delete_asset(full_asset_path)
    
    # Create import task
    import_task = unreal.AssetImportTask()
    import_task.filename = xml_path
    import_task.destination_path = asset_path
    import_task.destination_name = asset_name
    import_task.replace_existing = True
    import_task.automated = True
    import_task.save = True
    
    # Get the factory
    factory = unreal.BehaviacBehaviorTreeImportFactory()
    import_task.factory = factory
    
    unreal.log_warning(f"⚙️  Importing {asset_name} from XML...")
    
    # Execute import
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([import_task])
    
    # Check if import succeeded
    if import_task.imported_object_paths and len(import_task.imported_object_paths) > 0:
        imported_path = import_task.imported_object_paths[0]
        unreal.log_warning(f"✅ Successfully imported: {imported_path}")
        
        # Verify the asset exists and save it
        if unreal.EditorAssetLibrary.does_asset_exist(imported_path):
            asset = unreal.EditorAssetLibrary.load_asset(imported_path)
            unreal.EditorAssetLibrary.save_asset(imported_path)
            unreal.log_warning(f"💾 Saved asset: {imported_path}")
            return True
        else:
            unreal.log_error(f"❌ Asset not found after import: {imported_path}")
            return False
    else:
        unreal.log_error(f"❌ Import failed - no objects imported")
        return False

def main():
    """Main entry point - expects XML file path as argument"""
    import sys
    
    if len(sys.argv) < 2:
        unreal.log_error("Usage: reimport_bt.py <xml_file_path> [asset_name]")
        return
    
    xml_path = sys.argv[1]
    
    # Extract asset name from XML filename if not provided
    if len(sys.argv) >= 3:
        asset_name = sys.argv[2]
    else:
        asset_name = os.path.splitext(os.path.basename(xml_path))[0]
    
    unreal.log_warning(f"🔄 Starting import process")
    unreal.log_warning(f"   XML: {xml_path}")
    unreal.log_warning(f"   Asset: {asset_name}")
    
    success = import_behavior_tree_from_xml(xml_path, asset_name)
    
    if success:
        unreal.log_warning(f"✅ Import completed successfully!")
    else:
        unreal.log_error(f"❌ Import failed!")
    
    return success

if __name__ == "__main__":
    main()
