# PushupCounter - A Walabot Application

A simple Walabot application that acts as your personal push-up trainer.  
Choose a difficulty level, calibrate your utmost position, and start pushing!  

* The app was tested on Windows 10, compiled using Visual Studio.
* Sound files are playing using the [Windows API](https://msdn.microsoft.com/en-us/library/windows/desktop/dd743680.aspx).

### What does the Walabot Do?

* The app uses the Walabot sensor to detect the height of a person above it.
* The YZ plane is used to calculate the distance.

## How to use

1. Install the [Walabot SDK](http://walabot.com/getting-started).
2. [Build the project](http://api.walabot.com/_project.html).
3. Start pushing! :muscle: :muscle:

**IMPORTANT NOTE:** Current Walabot settings are for vMaker18.

## Editing the code

At the top of the code you can find variables that can be changed easily without dealing with the "heavy" part of the code.  
All those variables should vary between different Walabot boards, operating systems, operating machines, etc.

* `double gBeginnerRep`: These values determine how deep you need to go during a push-up,
* `double gMediumRep`: as a percentage  of your upmost position,
* `double gAdvancedRep`: for it to count as a rep.
