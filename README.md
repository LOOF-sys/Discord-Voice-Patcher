# Discord Voice Patcher
- This tool was created by extensive analysis of discords voice node to force discord to be stereo, maxed bitrate, CELT mode, higher baseline volume and no FEC without javascript access for newer nodes which don't support it anymore
- Report audio quality problems or crashing/loading/sound not working in Issues

## Usage
- Before running this tool at all, go into your discord's modules voice directory, by default its located at C:\Users\PUT YOUR WINDOWS USER HERE\AppData\Local\Discord, then go into app-x.x.xxxx\modules, then go into the discord_voice-x (means number so example "discord_voice-1") folder
- Find the folder inside the folder you are in called "discord_voice", not "discord_voice-x", it should just be called exactly "discord_voice", replace this folder with the folder located inside the zip after you have extracted it, the folder should not contain a folder with the the name "discord_voice", if it does you did not follow instructions, it should contain the "discord_voice.node" file
- Run the tool when you launch discord and it will automatically patch your discord process

## Compiling
- Go into Visual Studio and create a console application, copy all of the source in the discordpatcher.cpp file and override the default .cpp file's contents and create a new c file and name it patch.c and copy all of the patch.c text into the file
- Properties > C/C++ > Optimization set Optimization to favor size and set "Favor size or speed" to small code
- Set the project to release 

![image](https://github.com/user-attachments/assets/4a1df9bc-46d6-4b88-a8b9-69bd38bb09da)
- Change the programs character set to "Not Set" or "Multi-byte" and it will compile

![image](https://github.com/user-attachments/assets/cb442aa5-2e08-42e5-83ae-7de702b01005)
