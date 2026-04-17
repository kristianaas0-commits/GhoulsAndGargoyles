import unreal


ASSET_PATH = "/Game/Enemies/BP_enemy"


def main():
    asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if not asset:
        raise RuntimeError(f"Failed to load blueprint asset: {ASSET_PATH}")

    if not isinstance(asset, unreal.Blueprint):
        raise RuntimeError(f"Asset is not a Blueprint: {ASSET_PATH}")

    new_parent_class = unreal.load_class(None, "/Script/GnG.EnemyParent")
    if not new_parent_class:
        raise RuntimeError("Failed to load /Script/GnG.EnemyParent")

    unreal.BlueprintEditorLibrary.reparent_blueprint(asset, new_parent_class)
    unreal.BlueprintEditorLibrary.compile_blueprint(asset)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    unreal.log("Reparented BP_enemy to EnemyParent and saved the asset.")


if __name__ == "__main__":
    main()
