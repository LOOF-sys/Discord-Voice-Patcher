# Discord Voice Patcher
- **Dynamic discord version support will be added next update for ease of use and more support for a variety of discord versions**
- At this point this is a finished product and will not recieve anymore updates unless it is something required (maybe signature scanning in the future), gimmicks like useless audio control pannels don't count as required
- This tool was created by extensive analysis of discords voice node to force discord to be stereo, maxed bitrate, CELT mode, higher baseline volume and no FEC without javascript access for newer nodes which don't support it anymore
- Report audio quality problems or crashing/loading/sound not working in Issues

## Usage
- Video Tutorial https://www.youtube.com/watch?v=2DuN9YEccVk
- Make sure to download the discord_voice.zip file from this repository before continuing
- Before running this tool at all, go into your discord's modules voice directory, by default its located at C:\Users\PUT YOUR WINDOWS USER HERE\AppData\Local\Discord, then go into app-x.x.xxxx\modules, then go into the discord_voice-x (means number so example "discord_voice-1") folder
- Find the folder inside the folder you are in called "discord_voice", not "discord_voice-x", it should just be called exactly "discord_voice", replace this folder with the folder located inside the zip after you have extracted it, the folder should not contain a folder with the the name "discord_voice", if it does you did not follow instructions, it should contain the "discord_voice.node" file
- Run the tool when you launch discord and it will automatically patch your discord process

## Compiling (Do in order)
- Go into Visual Studio and create a console application, copy all of the source in the discordpatcher.cpp file and override the default .cpp file's contents and create a new c file and name it patch.cpp and copy all of the patch.cpp text into the file
- Set the project to release
- Properties > C/C++ > Optimization set Optimization to favor size and set "Favor size or speed" to small code **(Has to be done or it will crash)**

![image](https://github.com/user-attachments/assets/4a1df9bc-46d6-4b88-a8b9-69bd38bb09da)
- Change the programs character set to "Not Set" or "Multi-byte" and it will compile

![image](https://github.com/user-attachments/assets/cb442aa5-2e08-42e5-83ae-7de702b01005)
