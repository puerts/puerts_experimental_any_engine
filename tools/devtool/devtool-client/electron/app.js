const { app, BrowserWindow, Menu, MenuItem, ipcMain } = require('electron')
const { join } = require('path')
const { registerProtocol } = require('./protocol')
if (!process.cwd().endsWith('devtool')) throw new Error('must run in devtool/ directory')
const createClient = require(join(process.cwd(), 'cdp'))

app.whenReady().then(() => {
  registerProtocol();
  ipcMain.handle('connectInspector', function (event, url) {
    event.sender.loadFile(join(__dirname, './front_end.asar/inspector.html'), {
      query: {
        ws: url
      }
    })
    let client = null;
    event.sender.on('did-finish-load', async () => {
      try {
        client?.close({});
  
        client = await createClient(...url.split(':'));
      } catch(e) {
        console.error(e);
      }
    })
  })

  function createWindow() {
    const win = new BrowserWindow({
      width: 1280,
      height: 720,
      webPreferences: {
        preload: join(__dirname, 'preload.js')
      }
    })

    const menu = new Menu();
    [
      new MenuItem({
        label: 'Open New...',
        click() {
          createWindow();
        }
      }),
      new MenuItem({
        label: 'Reload',
        click() {
          win.webContents.reload()
        }
      }),
      new MenuItem({
        label: '[Develop Mode]',
        role: 'toggleDevTools'
      })
    ].forEach((menuItem, index) => {
      menu.insert(index, menuItem)
    })
    win.setMenu(menu);

    win.loadURL(join(__dirname, 'index.html'));
  }

  createWindow()

  app.on('window-all-closed', () => {
    process.exit()
  })
})