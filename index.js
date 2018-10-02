const hooker = require('./build/Debug/nexthooker')

// testing まほ×ろば -Witches spiritual home-
const PID = 6396 //pid of game
const H_CODE = '/HWN-4@83AAB:mahoXroba.exe' //H-CODE

hooker.onProcessDetach((pid) => { console.log(`process detached: ${pid}`) })
hooker.onProcessAttach((pid) => { console.log(`process attached: ${pid}`) })
hooker.onThreadCreate(
    (ts) => { console.log(`thread create: ${JSON.stringify(ts)}`) },
    (ts, text) => { console.log(`get text '${text}' from thread: ${JSON.stringify(ts)}`) }
)
hooker.onThreadRemove((ts) => { console.log(`thread removed: ${JSON.stringify(ts)}`) })
hooker.start()
hooker.injectProcess(PID)
setTimeout(() => {
    hooker.insertHook(PID, H_CODE)
}, 1000);
