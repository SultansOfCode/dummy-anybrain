# Dummy Anybrain DLL

## Disclaimer

The purpose of this DLL is to bypass the protection mechanisms from the Anybrain's company for the game Ravendawn. Maybe it will work with other games, but it is untested

## How to use

You will need the compiled DLL file. Either from the sources, or directly from this repository in the release section (it is not guaranteed that the `.dll` here is kept up to date with the sources)

Also you will need to set up the configuration file. The documentation for its configuration are inside it and are pretty straight forward

Go to the game's installation folder. Go inside the sounds folder. Identify which file there is a PE32+ file (at the time of writing this document, it is `wooby_sound.bnk`). Rename it to `.dll` instead of `.bnk`. Paste inside this folder the dummy DLL from this repository and the configuration INI file. Rename them as the original name (so, i.e. `wooby_sound.bnk` for the dummy DLL and `wooby_sound.ini` for the configuration INI when the original DLL is `wooby_sound.bnk`)

There are two methods to bypass the protection right now (they also were not extensively tested, so use with caution and all the ban risks are on you)

## Method 1

You will need a patched client that allows you to run more than one instance of it. It can be got in the other repository from this account. The live version of it can be found here: [ReverseCRC](https://sultansofcode.github.io/ReverseCRC/web)

Also you will need two characters in the same game's account

The INI file should have these properties:

```
...

[skipOriginal]
AnybrainStartSDK=0
AnybrainPauseSDK=0
AnybrainResumeSDK=0
AnybrainStopSDK=0
AnybrainSetCredentials=0
AnybrainSetUserId=0

[fakeUserId]
enabled=0

...
```

Open one instance of the game and login with the character that will not be running bots or anything. This will send the correct user id to their servers and the game will be able to check that you are conected without any cheats

Then modify the INI file as following:

```
...

[fakeUserId]
enabled=1
arg0=test|123456

...
```

Save it and run a second isntance of the game. This time, when you login, the user id will be patched. You can do whatever you want with this account now, because the server will think that the correct user id for the account is legitimately connected to Anybrain's server due to the first opened client

Keep playing with the first client, just to keep real data going to Anybrain for the original user id

## Method 2

This method pauses and resumes the Anybrain's protection automatically after its initialization

Just configure the INI file to enable it and set the time you want the protection to be on and off:

```
...

[autoPauseResume]
enabled=1
timeBeforePause=30000
timeBeforeResume=60000

...
```

This configuration will let the protection run for 30 seconds and pause it for 1 minute

## Debug

There are some debugger flags in the INI file, so you can see what is going on. Maybe more will be added in the future (maybe not)

## TODO

- Research Anybrain's network traffic. Until now we only know that it skips `/etc/hosts` file to resolve their server's name
- Research if we can just skip the data collection instead of pausing/resuming the entire protection
- Test it a lot
