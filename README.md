# CPLauncher
CPLauncher is an application that allows you to launch applications with your keyboard.
Currently the application is in early development stage, that is why errors may occur, and something can work not as expected.
The application is made using cross platform C++ GUI framework
QT 5.15, that is why potentially it can work with some tweaks on many platforms that are supported by QT,
but only for **Windows** and **OSX** platform specific functions were implemented right now.

### Features
* Run applications and files from specified folders
* Open bookmarks from Chrome/Firefox browsers
* Snippets with placeholders
* Custom web search
* Integrated calculator
* Custom Workflows that allow you to program actions with blocks and arrows 

### Installation
#### Windows
* Download the lastest zip release from [Releases](https://github.com/cplauncher/CPLauncher/releases/)
* Unzip it
* Install [Visual Studio redistributable X86](https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) in case if it is not installed yet. [Direct link](https://aka.ms/vs/17/release/vc_redist.x86.exe)
* Run
#### Mac
* Download the lastest zip release from [Releases](https://github.com/cplauncher/CPLauncher/releases/)
* Unzip it
* Run

### Usage
After starting the application, you will see the tray icon in tray area. There you will be able to choose **Show Settings**.
In **General** tab you will be able to configure the toogle CPLauncher hotkey(by default it is Ctrl+Space).
<p align="center">
  <img src="https://raw.githubusercontent.com/cplauncher/CPLauncher/master/readme_resources/cplauncher_input_dialog.png" alt="Main CPLauncher input dialog" width="613" height="481" />
</p>

In this window you can type part of your application name and if it is present in indexed folders Settings window **File System** tab.

The application can much more that just run applications from specified folders.

It can open bookmarks from Chrome and Firefox if you allow it in **Browser Bookmarks** settings

You can configure custom web search. Web search allows you to quickly run search on websites
<p align="center">
  <img src="https://raw.githubusercontent.com/cplauncher/CPLauncher/master/readme_resources/websearch_settings.png" alt="Websearch settings" width="588" height="333" />
</p>

With **Snippets** settings you can manage some small text clips.
Snippets are organized in collections, and each collection may have own activation keyword(or you can leave it empty to show snippets in root search)  
Snippets are not completely static chunks of text. You can add placeholders that will be expanded in run time. Placeholders can be recursively nested in each other.
<p align="center">
  <img src="https://raw.githubusercontent.com/cplauncher/CPLauncher/master/readme_resources/snippets_settings.png" alt="Snippets settings" width="873" height="600" />
</p>

With **Workflows** you will be able to configure some sequence of actions that you can run in several ways(keyword in input dialog/hotkey/others)
<p align="center">
  <img src="https://raw.githubusercontent.com/cplauncher/CPLauncher/master/readme_resources/workflow_settings.png" alt="Workflow settings" width="650" height="563" />
</p>
Right now the list of blocks accessible in workflow is not very big, but I hope it will grow.


### How to Debug
*TODO: make the build not depend on QtCreator*

* Clone the repository with:

*git clone git@github.com:cplauncher/CPLauncher.git*

*git clone git@github.com:itay-grudev/SingleApplication.git CPLauncher/libs/projects/SingleApplication*

*git clone git@github.com:Skycoder42/QHotkey.git CPLauncher/libs/projects/QHotkey*

*git clone git@github.com:edubart/minilua.git CPLauncher/libs/projects/lua/minilua*

* Open in QtCreator and run.

### How to build

* Make the all steps from **How to Debug** section

* Make release build in QtCreator

* In terminal navigate to build-CpLauncher*-Release folder

* Set environment variable QT_DIR to your compiler QT folder. For example "d:\Tools\QT\5.15.2\msvc2019" or "C:\QT\5.15.2\mingw81_64"

* Execute script **../CpLauncher/build_windows.bat** or **sh ../CpLauncher/build_osx.sh**

* As a result you will receive CpLauncher folder with all necessary libraries