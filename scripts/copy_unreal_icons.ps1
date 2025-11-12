# PowerShell script to copy Unreal Engine transform tool icons
# Run this after cloning: git clone https://github.com/EpicKiwi/unreal-engine-editor-icons.git

$unrealIconsPath = "..\unreal_icons\imgs\Icons"
$targetPath = "..\assets\icons"

if (Test-Path $unrealIconsPath) {
    Write-Host "Copying Unreal Engine icons..."
    
    # Create target directory if it doesn't exist
    if (-not (Test-Path $targetPath)) {
        New-Item -ItemType Directory -Path $targetPath -Force | Out-Null
    }
    
    # Copy translate icon (try different sizes)
    $translateFiles = @("icon_translate_40x.png", "icon_translate_24x.png", "icon_translate_16x.png", "icon_translateb_16x.png")
    foreach ($file in $translateFiles) {
        $source = Join-Path $unrealIconsPath $file
        if (Test-Path $source) {
            Copy-Item $source $targetPath -Force
            Write-Host "Copied: $file"
            break
        }
    }
    
    # Copy rotate icon (try different sizes)
    $rotateFiles = @("icon_rotate_40x.png", "icon_rotate_24x.png", "icon_rotate_16x.png", "icon_rotateb_16x.png")
    foreach ($file in $rotateFiles) {
        $source = Join-Path $unrealIconsPath $file
        if (Test-Path $source) {
            Copy-Item $source $targetPath -Force
            Write-Host "Copied: $file"
            break
        }
    }
    
    # Copy scale icon (try different sizes)
    $scaleFiles = @("icon_scale_40x.png", "icon_scale_24x.png", "icon_scale_16x.png", "icon_scaleb_16x.png")
    foreach ($file in $scaleFiles) {
        $source = Join-Path $unrealIconsPath $file
        if (Test-Path $source) {
            Copy-Item $source $targetPath -Force
            Write-Host "Copied: $file"
            break
        }
    }
    
    Write-Host "Done! Icons copied to $targetPath"
} else {
    Write-Host "Error: Unreal icons directory not found at $unrealIconsPath"
    Write-Host "Please clone the repository first:"
    Write-Host "  git clone https://github.com/EpicKiwi/unreal-engine-editor-icons.git unreal_icons"
}

