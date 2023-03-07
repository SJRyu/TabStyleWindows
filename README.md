# TabStyleWindows
Win32 + Winui2 Compositor and Multithreading - A thread per tab.

Sample is a viewer for ipcams - But actual connection and playing is ditched.
You can see basic operations on windows and dialog with that.

<img width="75%" src="https://user-images.githubusercontent.com/18696849/222093537-2aea3d23-dbba-4ab9-8228-76cfc022af72.PNG">

deved with VS2022, C++17, x64.

# WinUI Composition VS DirectComposition
In a brief, DComp is just better. Modern webbrowsers use dcomp as its backend on Windows. Ofc, At the time when they developed, C++/WinRT didn't even exist. 
But regardless of that fact, WinUI is still not able to be a better option. Cuz It has got restriced in threading, which is huge difference between dcomp and winui. 
With WinUI Composition, as a pros and cons at sametime, You are not allowed to program your own threading and dispatch model for composition batch/commit operations, in the otherwords, You are not allowed to make threading per each visual, which is an huge defect.

For an example, in this project, I devide composition windows into D2dWindow and D2dWindow1.
Cuz I can't use single visual tree when i make multithread for windows and their visuals. And My project probably use more threads and resources by the winui, compared to using single visual tree with dcomp.
