const { readFileSync, existsSync } = require('fs')
const { join } = require('path')

let contentsPath = join(process.cwd(), '../.contents');
if (process.argv.indexOf('--contents-path') != -1) {
    const contentsPathToCWD = process.argv[process.argv.indexOf('--contents-path') + 1];
    const contentsPathTemp = join(process.cwd(), contentsPathToCWD)
    if (existsSync(contentsPathTemp)) {
        contentsPath = contentsPathTemp
    } else {
        throw new Error('invalid contents path ' + contentsPathTemp)
    }
}


module.exports = function ({ url }) {
    const allowProtocolAndHost = 'devtool://contents/';
    if (!url.startsWith(allowProtocolAndHost)) return {}
    const relativePath = (new URL(url)).pathname.slice(1)
    
    let content = readFileSync(
        join(contentsPath, relativePath), 'utf-8'
    );

    return {
        url,
        content
    }
}