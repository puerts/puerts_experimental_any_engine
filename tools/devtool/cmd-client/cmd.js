const host = process.argv[2]
const port = +process.argv[3]
const createClient = require('../cdp')

    ;
(async function () {
    const client = await createClient(host, port);
    const { Runtime } = client

    let testResultPromise = new Promise(resolve => {
        let testReportStarted = false
        let testReportEnded = false
        Runtime.on('consoleAPICalled', params => {
            if (params.type == "log") {
                if (params.args[0].type == 'string' && params.args[0].value == "=== Puerh Test Started ===") {
                    testReportStarted = true;
                }
                if (params.args[0].type == 'string' && params.args[0].value.indexOf('%d passing') != -1) {
                    passed = params.args[1].value;
                    setTimeout(resolve, 200)
                    testReportEnded = true;
                }
                if (params.args[0].type == 'string' && params.args[0].value.indexOf('%d failing') != -1) {
                    failed = params.args[1].value;
                    setTimeout(resolve, 200)
                    testReportEnded = true;
                }
                if (testReportStarted && !testReportEnded) {
                    console.log(params.args[0].value, ...params.args.slice(1).map(i => i.value));
                }
            }
        })
    });
    let failed = 0;
    let passed = 0;
    const uniqueContextIdPromise = new Promise(resolve => {
        Runtime.on("executionContextCreated", (params) => {
            resolve(params.context.uniqueId)
        })
    })
    await Runtime.enable();

    await Runtime.evaluate({
        expression: 'runUT()',
        replMode: true,
        disableBreaks: true,
        throwOnSideEffect: false,
        awaitPromise: true,
        uniqueContextId: await uniqueContextIdPromise
    });

    await testResultPromise
    console.log(`run done passed ${passed} failed ${failed}`)
    process.exit(0)

})().catch(e => {
    console.error(e);
    process.exit(1)
});