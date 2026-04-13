# DesktopInvasion -- [Play for free](https://www.desktopinvasion.com)!
This is the public repository for DesktopInvasion: a fanmade Pokémon desktop pet/game that allows you to battle, catch & play with the beautiful Heartgold & Soulsilver overworld sprites, whenever & wherever you go on your pc! It's a cute desktop pet that also implements a "stub version" of generation-4 Pokémon. Check out the [trailer](https://www.youtube.com/watch?v=Z9CHE52CrkE)!

If you have any questions about DesktopInvasion, please contact me at desktopinvasion@gmail.com

## Download
Available for Windows, Mac, and Linux (X11/XWayland required). Get it [here](https://www.desktopinvasion.com/?page=download).

## Credits
No assets used in the game are included in this repository. DesktopInvasion is a non-commercial fan project that is available for free. I have never and will never seek any fees or donations for this game. This game makes extensive use of material that I do not own. It is my sincere belief that I have used this material in a spirit of fair use. Apart from moderate use of LLMs, all source code is written by me, with gameplay based on widely available sources. See more [here](https://desktopinvasion.com/?page=credits).

Game assets were primarily taken from [Spriter's Resource](https://www.spriters-resource.com/ds_dsi/pokemonheartgoldsoulsilver/). Assets were manually cleaned up and reformatted, and then preprocessed in a custom format using Python scripts. Scripts were also used to extract game data from [pokeapi](https://pokeapi.co/). Assets and game data are not included in this repo.

## Development
DesktopInvasion is built with Qt 6.8 and CMake/Ninja. There is a Docker-based development environment with reload on source changes and Valgrind for memory analysis. For release builds, GitHub actions was used, with windeployqt for Windows and macdeployqt for Mac, whereas for Linux an .AppImage binary was built locally using linuxdeploy with the Qt plugin

