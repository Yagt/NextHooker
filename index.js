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