# NextHooker Node.js Addon

## Overview

node-nexthooker is a Node.js wrapper for NextHooker.

[NextHooker](https://github.com/Artikash/NextHooker) is a GUI text hooker based on [Stomp](http://www.hongfire.com/forum/showthread.php/438331-ITHVNR-ITH-with-the-VNR-engine)'s ITHVNR.

## **WARNING**

This addon could only work well with Electron!

On original Node.js, no text will be get by the core lib.

Don't know why for now, may be fixed in future.

## Usage (with Electron)

### Get the latest version

    npm install --save-dev electron
    npm install --save-dev nexthooker --force

if you see an error with *node-gyp rebuild*, don't worry.

### Build library

    cd .\node_modules\nexthooker

1. Open CMakeLists.txt from Visual Studio

2. CMake -> Generate All

### Rebuild for Electron

    npm install -g node-gyp
    node-gyp rebuild --target=<ELECTRON_VERSION> --arch=ia32 --dist-url=https://atom.io/download/electron --debug

### Copy Library to Build Folder

    cp ./Builds/Debug/Debug/vnrhook.dll ./build/Debug && cp ./Builds/Debug/Debug/vnrhost.dll ./build/Debug

### Create test program

    const hooker = require('nexthooker/build/Debug/nexthooker')
    hooker.start()
    hooker.onProcessDetach((pid) => { console.log(`process detached: ${pid}`) })
    hooker.onProcessAttach((pid) => { console.log(`process attached: ${pid}`) })
    hooker.onThreadCreate(
        (ts) => { console.log(`thread create: ${ts}`) },
        (ts, text) => { console.log(`get text '${text}' from thread: ${ts}`) }
    )
    hooker.onThreadRemove((ts) => { console.log(`thread removed: ${ts}`) })
    hooker.open()
    hooker.injectProcess(parseInt(process.argv[2]))
    /* index.js */

### Run test program

    .\node_modules\.bin\electron.cmd index.js <PID_OF_GAME>

## ~~Usage (with Node.js)~~

### Get the latest version

    npm install nexthooker

or

    git clone https://github.com/Yagt/node-nexthooker

### Build library

1. Open CMakeLists.txt from Visual Studio
2. CMake -> Generate All

### Rebuild addon

    npm install -g node-gyp
    node-gyp rebuild --debug && cp ./Builds/Debug/Debug/vnrhook.dll . && cp ./Builds/Debug/Debug/vnrhost.dll .

### Create test program

    const hooker = require('./build/Debug/nexthooker')
    hooker.start()
    hooker.onProcessDetach((pid) => { console.log(`process detached: ${pid}`) })
    hooker.onProcessAttach((pid) => { console.log(`process attached: ${pid}`) })
    hooker.onThreadCreate(
        (ts) => { console.log(`thread create: ${ts}`) },
        (ts, text) => { console.log(`get text '${text}' from thread: ${ts}`) }
    )
    hooker.onThreadRemove((ts) => { console.log(`thread removed: ${ts}`) })
    hooker.open()
    hooker.injectProcess(parseInt(process.argv[2]))
    /* index.js */

### Run test program

    node index.js <PID_OF_GAME>

## License

GPL v3
