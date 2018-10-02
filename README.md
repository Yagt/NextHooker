# NextHooker Node.js Addon

## Overview

node-nexthooker is a Node.js wrapper for NextHooker (now renamed Textractor).

[Textractor](https://github.com/Artikash/Textractor) is the next generation of text hooker made by [Artikash](https://github.com/Artikash/) and [DoumanAsh](https://github.com/DoumanAsh/).

## **WARNING**

**For the best compatibility, vnrhook.dll and nexthooker.node SHOULD BE PLACED UNDER THE ROOT FOLDER OF YOUR PROJECT!!!**

## Usage (with Electron)

### Get the latest version

    npm install --save electron
    npm install --save nexthooker --force

if you see an error with _node-gyp rebuild_, don't worry.

### Build library

    cd .\node_modules\nexthooker

1. Open CMakeLists.txt from Visual Studio
2. Change build option to x86-Release
3. CMake -> Generate All

### Rebuild for Electron

    npm install -g node-gyp
    node-gyp rebuild --target=<ELECTRON_VERSION> --arch=ia32 --dist-url=https://atom.io/download/electron

### Copy Library to ROOT Folder (IMPORTANT)

    cp .\Builds\x86-Release\Build\vnrhook.dll ..\..
    cp .\build\Release\nexthooker.node ..\..

### Create test program

    const hooker = require('./nexthooker')
    hooker.onProcessAttach((pid) => { console.log(`process attached: ${pid}`) })
    hooker.onProcessDetach((pid) => { console.log(`process detached: ${pid}`) })
    hooker.onThreadCreate(
        (thread) => { console.log(`thread create: ${JSON.stringify(thread)}`) },
        (thread, text) => { console.log(`get text '${text}' from thread: ${JSON.stringify(thread)}`) }
    )
    hooker.onThreadRemove((thread) => { console.log(`thread removed: ${JSON.stringify(thread)}`) })
    hooker.start()
    hooker.injectProcess(parseInt(process.argv[2]))
    /* index.js */

### Run test program

    .\node_modules\.bin\electron.cmd index.js <PID_OF_GAME>

## Usage (with Node.js)

### Get the latest version

    npm install --save nexthooker --force

or

    git clone https://github.com/Yagt/node-nexthooker

### Build library

1. Open CMakeLists.txt from Visual Studio
2. Change build option to x86-Release
3. CMake -> Generate All

### Rebuild addon

    npm install -g node-gyp
    node-gyp rebuild

### Copy Library to ROOT Folder (IMPORTANT)

    cp .\Builds\x86-Release\Build\vnrhook.dll .
    cp .\build\Release\nexthooker.node .

### Create test program

    const hooker = require('./nexthooker')
    hooker.onProcessAttach((pid) => { console.log(`process attached: ${pid}`) })
    hooker.onProcessDetach((pid) => { console.log(`process detached: ${pid}`) })
    hooker.onThreadCreate(
        (thread) => { console.log(`thread create: ${JSON.stringify(thread)}`) },
        (thread, text) => { console.log(`get text '${text}' from thread: ${JSON.stringify(thread)}`) }
    )
    hooker.onThreadRemove((thread) => { console.log(`thread removed: ${JSON.stringify(thread)}`) })
    hooker.start()
    hooker.injectProcess(parseInt(process.argv[2]))
    /* index.js */

### Run test program

    node index.js <PID_OF_GAME>

## Assertion of Using

The four callback function (onProcessAttach, onProcessDetach, onThreadCreate and onThreadRemove) should be declared before start() being called.

## License

GPL v3
