# Discord Voice Patcher
- Developed by cypher at https://www.youtube.com/@cypher2226 or discord wcypher
- This tool was created by extensive analysis of discords voice node to force stereo to enable itself even though its disabled in the core without javascript access
- The actual patcher was made in 5 minutes however the reversing took a couple weeks

## Usage
- Before running this tool at all, go into your discord's modules voice directory, by default its located at C:\Users\PUT YOUR WINDOWS USER HERE\AppData\Local\Discord, then go into app-x.x.xxxx\modules, then go into the discord_voice-x (means number so example "discord_voice-1") folder
- Find the folder inside the folder you are in called "discord_voice", not "discord_voice-x", it should just be called exactly "discord_voice", replace this folder with the folder located inside the zip after you have extracted it, the folder should not contain a folder with the the name "discord_voice", if it does you did not follow instructions, it should contain the "discord_voice.node" file
- Run the tool when you launch discord and it will automatically patch your discord process
- This tool is intended for people who want to make their discord stereo and increase base volume of discord to remove any limits of the volume (program does that by default), it also removes the high pass filter and changes your audio encoder to CELT

## Can this be made permanet?
- Yes and soon I will add an auto-patcher to automatically do it for you
- **(File signing problem)** If you are having trouble for some reason with the files not being signed, use signtool or if this is some kind of check related thing, overlap the file using windows file system tricks and emulate its expected behavior

## Compiling
- Go into Visual Studio and create a console application, copy all of the source in the discordpatcher.cpp file and override the default .cpp file's contents
- Set the project to release 

![image](https://github.com/user-attachments/assets/4a1df9bc-46d6-4b88-a8b9-69bd38bb09da)
- Change the programs character set to "Not Set" or "Multi-byte" and it will compile

![image](https://github.com/user-attachments/assets/cb442aa5-2e08-42e5-83ae-7de702b01005)

## Based on what?
- This projects majority reversing was based off of Aethercord which was a previous client I used to host for the usage of "Loud micing" back in early-mid 2024 for people to loud mic on a "Loud list" without being told they were using invalid discord clients (everything that was not the electron discord app), so I made a hook that integrated into discord and named it Aethercord, then made a seperate electron client called Aethercord that hosted the appplication
- Inspiration for this project was to get rid of the need for injecting dll's and using modified discord executables to "get louder" on discord.com/listing
