# EldenRing Freecam Mod
A [mod](https://www.nexusmods.com/eldenring/mods/9420) that detaches camera from the player in Elden Ring. Currently freezes player while in camera mode.  
Works with Seamless Co-op, ER Reforged. Tested on EldenModLoader, modengine2, me3.  

![Freecam preview](https://github.com/user-attachments/assets/4d71527d-b1c3-4a38-9593-f1249f2c667e)
## Controls
You can find and change all keybinds in `config.ini`. If you are unsure about key's name use it's *0x* [hex code](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)
- **F1** – Toggle free camera  
- **W / A / S / D** – Move camera  
- **Shift / Space** – Move up / down  
- **Mouse Scroll** – Adjust camera speed  
- **Ctrl + Mouse Scroll (or + / -)** – Adjust FOV
- **Left Mouse Button** - Sprint

## Installation
1. Disable Easy Anti-Cheat using a tool such as [Anti-Cheat Toggler](https://www.nexusmods.com/eldenring/mods/90).
2. Install a DLL mod loader of your choice. (e.g. [EldenModLoader](https://www.nexusmods.com/eldenring/mods/117), [me3](https://github.com/garyttierney/me3)).
3. [Download](https://github.com/Logersnamed/FreecamMod/releases) the latest release and extract the contents of **Freecam.zip**:
   - For **EldenModLoader**: `...\steamapps\common\ELDEN RING\Game\mods`
   - For **me2/me3**: Specify the path in your profile configuration.
   - For other loaders, refer to their documentation.
5. Launch the game.

## Build
### Using CMake
```bash
git clone --recurse-submodule https://github.com/Logersnamed/FreecamMod.git
cd FreecamMod
```
Configure the project. Optionally you can specify a DLL output folder using DGAME_DIR variable:
```bash
cmake -S . -B build [-DGAME_DIR="path/to/modflolder/"]
```
Build the project:
```bash
cmake --build build --config Release
```
The built DLL will be located in: `build/Release/FreecamMod.dll`
### Using GitHub Actions
Prebuilt DLLs are available as workflow artifacts. Go to the Actions tab, select the latest run, and download the FreecamMod.dll artifact.

## Credits & References
[EROverlay](https://github.com/koalabear420/EROverlay) – Reference and partial code usage  
[EldenRing-PostureBarMod](https://github.com/Mordrog/EldenRing-PostureBarMod) – Reference  
[Techiew ModUtils](https://github.com/techiew/EldenRingMods/blob/master/ModUtils.h) - Elden ring mod utils  
[The Grand Archives](https://github.com/The-Grand-Archives/Elden-Ring-CT-TGA) - Cheat Table  
[Universal-WndProc-Hook](https://github.com/M0rtale/Universal-WndProc-Hook) – WndProc hooking library  
[MinHook](https://github.com/TsudaKageyu/minhook) – Function hooking library  
