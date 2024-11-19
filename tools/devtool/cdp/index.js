const CDP = require('chrome-remote-interface');

module.exports = async function connectClient(host, port) {
    if (isNaN(port)) throw new Error('invalid port')
    const client = await CDP({
        host,
        port,
        protocol: require('./protocol')
    });
    const { PuerhDebug } = client

    PuerhDebug.enthroneAsHost({
        callID: 0
    })

    PuerhDebug.on('hostCalled', async payload => {
        const { callID, action, params } = payload;

        let callError = '';
        let callResult = {};
        try {
            console.log('hostCalled: ' + action)

            callResult = require('./commands/' + action)(params) || {}

        } catch (e) {
            callError = e.stack

        }

        if (callError) {
            PuerhDebug.returnHostCall({
                callID,
                error: callError,
                result: null
            })

        } else {
            PuerhDebug.returnHostCall({
                callID,
                error: "",
                result: JSON.stringify(callResult)
            })

        }
    })

    return client;
}