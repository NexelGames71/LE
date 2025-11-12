# Unreal Engine Editor Icons

Place Unreal Engine editor-style icon images here. The system will automatically look for:

## Translate/Move Tool Icons (in order of preference):
- `icon_translate_40x.png` (40x40 pixels - recommended)
- `icon_translate_24x.png` (24x24 pixels)
- `icon_translate_16x.png` (16x16 pixels)
- `icon_translateb_16x.png` (16x16 pixels, blue variant)
- `translate.png` or `Translate.png` (fallback)

## Rotate Tool Icons (in order of preference):
- `icon_rotate_40x.png` (40x40 pixels - recommended)
- `icon_rotate_24x.png` (24x24 pixels)
- `icon_rotate_16x.png` (16x16 pixels)
- `icon_rotateb_16x.png` (16x16 pixels, blue variant)
- `rotate.png` or `Rotate.png` (fallback)

## Scale Tool Icons (in order of preference):
- `icon_scale_40x.png` (40x40 pixels - recommended)
- `icon_scale_24x.png` (24x24 pixels)
- `icon_scale_16x.png` (16x16 pixels)
- `icon_scaleb_16x.png` (16x16 pixels, blue variant)
- `scale.png` or `Scale.png` (fallback)

## Getting Unreal Engine Editor Icons

1. Clone the repository: `git clone https://github.com/EpicKiwi/unreal-engine-editor-icons.git`
2. Copy the following files from `unreal_icons/imgs/Icons/` to `assets/icons/`:
   - `icon_translate_40x.png` (or any size you prefer)
   - `icon_rotate_40x.png` (or `icon_rotateb_16x.png` if 40x doesn't exist)
   - `icon_scale_40x.png` (or `icon_scaleb_16x.png` if 40x doesn't exist)

Supported formats: PNG, JPG

If icon files are not found, the system will fall back to drawing the icons programmatically.
