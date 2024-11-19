const { protocol } = require('electron')
const { existsSync } = require('fs')
const { join } = require('path')
const loadDevtoolContents = require(join(process.cwd(), 'cdp/commands/loadDevtoolContents'))

let resolvers = {}
if (process.argv.indexOf('--resolver') != -1) {
    const resolverPathToCWD = process.argv[process.argv.indexOf('--resolver') + 1];
    const resolverPath = join(process.cwd(), resolverPathToCWD)
    if (existsSync(resolverPath)) {
        resolvers = require(resolverPath)
    } else {
        throw new Error('invalid resolver file ' + resolverPath)
    }
}

protocol.registerSchemesAsPrivileged([
    {
        scheme: 'devtool',
        privileges: {
            standard: true,
            secure: true,
            supportFetchAPI: true
        }
    },
    ...Object.keys(resolvers).map(resolverKey => {
        return {
            scheme: resolverKey,
            privileges: {
                standard: true,
                secure: true,
                supportFetchAPI: true
            }
        }
    })
])

exports.registerProtocol = function () {

    protocol.handle('devtool', request => {
        const res = loadDevtoolContents({ url: request.url })
        if (!res.content) return new Response("", {
            status: 404
        })
        return new Response(res.content, {
            status: 200,
            headers: { 'content-type': request.url.endsWith('js') ? 'application/javascript' : 'application/json' }
        })
    })
    Object.keys(resolvers).forEach(key => {
        protocol.handle(key, resolvers[key])
    })
}