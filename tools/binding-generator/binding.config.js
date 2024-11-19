const { join, resolve } = require('path')
const projectRootPath = "<project root>"

const defines = []
const includePaths = []

export default {
    "output": {
        // manually maintaining file for including all binding.
        "puerHeaderName": "AllBinding.h",
        // the generate position.
        "dts": "<place dts file>",
        "binding": "<place binding file>"
    },
    "base": projectRootPath,
    defines,
    includePaths,
    "headers": [
        join(projectRootPath, "./Main.h"),
    ],
    "includes": [
    ],
    // all the api with the usage of these namespace/type/member would not generated
    "refExcludes": {
        "namespaces": [
        ],
        "types": [
        ],
        "members": [
        ]
    },
    // these namespace/type/member itself would not generated
    // but it won't let the api with the usage of these namespace/type/member be not generatged
    // useful for something you want to write binding manually.
    "genExcludes": {
        "namespaces": [
        ],
        "types": [
        ],
        "members": [
        ]
    },
    // specify the dts name for some special class
    "specialTSNames": {
        "String": "string"
    }
}