const { join } = require('path')
const { writeFileSync } = require('fs')
const mkdirp = require('mkdirp')

const testReportDir = join(__dirname, "../../.contents/testreports")

module.exports = function (params) {
    const { name, content, testID } = params;
    mkdirp.sync(join(testReportDir, testID))
    writeFileSync(join(testReportDir, testID, name + '.txt'), content)
}