const { contextBridge, ipcRenderer } = require('electron')

contextBridge.exposeInMainWorld('titanInspector', {
    connect: (url) => {
        ipcRenderer.invoke('connectInspector', url)
    },
})