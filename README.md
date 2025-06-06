# Discord Voice Patcher
- Developed by cypher at https://www.youtube.com/@cypher2226 or discord wcypher
- This tool was created by extensive analysis of discords voice node to force stereo to enable itself even though its disabled in the core without javascript access
- The actual patcher was made in 5 minutes however the reversing took a couple weeks

## Usage
- Before running this tool at all, go into your discord's modules voice directory, by default its located at C:\Users\PUT YOUR WINDOWS USER HERE\AppData\Local\Discord, then go into app-x.x.xxxx\modules, then go into the discord_voice-x (means number so example "discord_voice-1") folder
- Find the folder inside the folder you are in called "discord_voice", not "discord_voice-x", it should just be called exactly "discord_voice", replace this folder with the folder located inside the zip after you have extracted it, the folder should not contain a folder with the the name "discord_voice", if it does you did not follow instructions, it should contain the "discord_voice.node" file
- Run the tool when you launch discord and it will automatically patch your discord process
- This tool is intended for people who want to make their discord stereo and increase base volume of discord to remove any limits of the volume (program does that by default)

## Can this be made permanet?
- Yes and if you are smart you would already know that just by looking at the source code
- No guides on doing this if ur brain work you can do it yourself

## Basic Spoofing for PC checks
Something to note is that yes I am planning on making some more additions later to make this harder to detect so that this can be widely used
- **(For file patching)** If you know windows file system with the knowledge of a peanut you would know that yes you can overlap files with fake exact copies and emulate basic functionality
- **(For process memory)** Listing these would give oink checkers a list of targets so not doing that, If you want one of these and have any of my various contact accounts, send a dm. I would recommend file patching if you don't know how to go about this one
- **(Evading Process Logs)** for this I would recommend concealing the programs logic (wink wink) inside another program that is capable of writing to other process memory (wink wink)
- **(Dealing with PC checking tools)** Using the process log evasion method you can easily circumvent any post checks
- **(Bypassing Int16 Based dB Checkers)** Fairly simple just hardclip your sound to [the correct value] using *Fruity Limiter* or something equivalent and leave the patcher at its defaults since it only uncaps you enough to barley reach the max

## Compiling
- Go into Visual Studio and create a console application, copy all of the source in the discordpatcher.cpp file and override the default .cpp file's contents
- Set the project to release 

![image](https://github.com/user-attachments/assets/4a1df9bc-46d6-4b88-a8b9-69bd38bb09da)
- Change the programs character set to "Not Set" or "Multi-byte" and it will compile

![image](https://github.com/user-attachments/assets/cb442aa5-2e08-42e5-83ae-7de702b01005)
