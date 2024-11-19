const protocol = {
    version: {
        major: '1',
        minor: '0'
    },
    domains: [{
        "domain": "Runtime",
        "description": "Runtime domain exposes JavaScript runtime by means of remote evaluation and mirror objects.\nEvaluation results are returned as mirror object that expose object type, string representation\nand unique identifier that can be used for further object reference. Original objects are\nmaintained in memory unless they are either explicitly released or are released along with the\nother objects in their object group.",
        "types": [
            {
                "id": "ScriptId",
                "description": "Unique script identifier.",
                "type": "string"
            },
            {
                "id": "RemoteObjectId",
                "description": "Unique object identifier.",
                "type": "string"
            },
            {
                "id": "UnserializableValue",
                "description": "Primitive value which cannot be JSON-stringified. Includes values `-0`, `NaN`, `Infinity`,\n`-Infinity`, and bigint literals.",
                "type": "string"
            },
            {
                "id": "RemoteObject",
                "description": "Mirror object referencing original JavaScript object.",
                "type": "object",
                "properties": [
                    {
                        "name": "type",
                        "description": "Object type.",
                        "type": "string",
                        "enum": [
                            "object",
                            "function",
                            "undefined",
                            "string",
                            "number",
                            "boolean",
                            "symbol",
                            "bigint"
                        ]
                    },
                    {
                        "name": "subtype",
                        "description": "Object subtype hint. Specified for `object` type values only.\nNOTE: If you change anything here, make sure to also update\n`subtype` in `ObjectPreview` and `PropertyPreview` below.",
                        "optional": true,
                        "type": "string",
                        "enum": [
                            "array",
                            "null",
                            "node",
                            "regexp",
                            "date",
                            "map",
                            "set",
                            "weakmap",
                            "weakset",
                            "iterator",
                            "generator",
                            "error",
                            "proxy",
                            "promise",
                            "typedarray",
                            "arraybuffer",
                            "dataview",
                            "webassemblymemory",
                            "wasmvalue"
                        ]
                    },
                    {
                        "name": "className",
                        "description": "Object class (constructor) name. Specified for `object` type values only.",
                        "optional": true,
                        "type": "string"
                    },
                    {
                        "name": "value",
                        "description": "Remote object value in case of primitive values or JSON values (if it was requested).",
                        "optional": true,
                        "type": "any"
                    },
                    {
                        "name": "unserializableValue",
                        "description": "Primitive value which can not be JSON-stringified does not have `value`, but gets this\nproperty.",
                        "optional": true,
                        "$ref": "UnserializableValue"
                    },
                    {
                        "name": "description",
                        "description": "String representation of the object.",
                        "optional": true,
                        "type": "string"
                    },
                    {
                        "name": "objectId",
                        "description": "Unique object identifier (for non-primitive values).",
                        "optional": true,
                        "$ref": "RemoteObjectId"
                    },
                    {
                        "name": "preview",
                        "description": "Preview containing abbreviated property values. Specified for `object` type values only.",
                        "experimental": true,
                        "optional": true,
                        "$ref": "ObjectPreview"
                    },
                    {
                        "name": "customPreview",
                        "experimental": true,
                        "optional": true,
                        "$ref": "CustomPreview"
                    }
                ]
            },
            {
                "id": "CustomPreview",
                "experimental": true,
                "type": "object",
                "properties": [
                    {
                        "name": "header",
                        "description": "The JSON-stringified result of formatter.header(object, config) call.\nIt contains json ML array that represents RemoteObject.",
                        "type": "string"
                    },
                    {
                        "name": "bodyGetterId",
                        "description": "If formatter returns true as a result of formatter.hasBody call then bodyGetterId will\ncontain RemoteObjectId for the function that returns result of formatter.body(object, config) call.\nThe result value is json ML array.",
                        "optional": true,
                        "$ref": "RemoteObjectId"
                    }
                ]
            },
            {
                "id": "ObjectPreview",
                "description": "Object containing abbreviated remote object value.",
                "experimental": true,
                "type": "object",
                "properties": [
                    {
                        "name": "type",
                        "description": "Object type.",
                        "type": "string",
                        "enum": [
                            "object",
                            "function",
                            "undefined",
                            "string",
                            "number",
                            "boolean",
                            "symbol",
                            "bigint"
                        ]
                    },
                    {
                        "name": "subtype",
                        "description": "Object subtype hint. Specified for `object` type values only.",
                        "optional": true,
                        "type": "string",
                        "enum": [
                            "array",
                            "null",
                            "node",
                            "regexp",
                            "date",
                            "map",
                            "set",
                            "weakmap",
                            "weakset",
                            "iterator",
                            "generator",
                            "error",
                            "proxy",
                            "promise",
                            "typedarray",
                            "arraybuffer",
                            "dataview",
                            "webassemblymemory",
                            "wasmvalue"
                        ]
                    },
                    {
                        "name": "description",
                        "description": "String representation of the object.",
                        "optional": true,
                        "type": "string"
                    },
                    {
                        "name": "overflow",
                        "description": "True iff some of the properties or entries of the original object did not fit.",
                        "type": "boolean"
                    },
                    {
                        "name": "properties",
                        "description": "List of the properties.",
                        "type": "array",
                        "items": {
                            "$ref": "PropertyPreview"
                        }
                    },
                    {
                        "name": "entries",
                        "description": "List of the entries. Specified for `map` and `set` subtype values only.",
                        "optional": true,
                        "type": "array",
                        "items": {
                            "$ref": "EntryPreview"
                        }
                    }
                ]
            },
            {
                "id": "PropertyPreview",
                "experimental": true,
                "type": "object",
                "properties": [
                    {
                        "name": "name",
                        "description": "Property name.",
                        "type": "string"
                    },
                    {
                        "name": "type",
                        "description": "Object type. Accessor means that the property itself is an accessor property.",
                        "type": "string",
                        "enum": [
                            "object",
                            "function",
                            "undefined",
                            "string",
                            "number",
                            "boolean",
                            "symbol",
                            "accessor",
                            "bigint"
                        ]
                    },
                    {
                        "name": "value",
                        "description": "User-friendly property value string.",
                        "optional": true,
                        "type": "string"
                    },
                    {
                        "name": "valuePreview",
                        "description": "Nested value preview.",
                        "optional": true,
                        "$ref": "ObjectPreview"
                    },
                    {
                        "name": "subtype",
                        "description": "Object subtype hint. Specified for `object` type values only.",
                        "optional": true,
                        "type": "string",
                        "enum": [
                            "array",
                            "null",
                            "node",
                            "regexp",
                            "date",
                            "map",
                            "set",
                            "weakmap",
                            "weakset",
                            "iterator",
                            "generator",
                            "error",
                            "proxy",
                            "promise",
                            "typedarray",
                            "arraybuffer",
                            "dataview",
                            "webassemblymemory",
                            "wasmvalue"
                        ]
                    }
                ]
            },
            {
                "id": "EntryPreview",
                "experimental": true,
                "type": "object",
                "properties": [
                    {
                        "name": "key",
                        "description": "Preview of the key. Specified for map-like collection entries.",
                        "optional": true,
                        "$ref": "ObjectPreview"
                    },
                    {
                        "name": "value",
                        "description": "Preview of the value.",
                        "$ref": "ObjectPreview"
                    }
                ]
            },
            {
                "id": "PropertyDescriptor",
                "description": "Object property descriptor.",
                "type": "object",
                "properties": [
                    {
                        "name": "name",
                        "description": "Property name or symbol description.",
                        "type": "string"
                    },
                    {
                        "name": "value",
                        "description": "The value associated with the property.",
                        "optional": true,
                        "$ref": "RemoteObject"
                    },
                    {
                        "name": "writable",
                        "description": "True if the value associated with the property may be changed (data descriptors only).",
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "get",
                        "description": "A function which serves as a getter for the property, or `undefined` if there is no getter\n(accessor descriptors only).",
                        "optional": true,
                        "$ref": "RemoteObject"
                    },
                    {
                        "name": "set",
                        "description": "A function which serves as a setter for the property, or `undefined` if there is no setter\n(accessor descriptors only).",
                        "optional": true,
                        "$ref": "RemoteObject"
                    },
                    {
                        "name": "configurable",
                        "description": "True if the type of this property descriptor may be changed and if the property may be\ndeleted from the corresponding object.",
                        "type": "boolean"
                    },
                    {
                        "name": "enumerable",
                        "description": "True if this property shows up during enumeration of the properties on the corresponding\nobject.",
                        "type": "boolean"
                    },
                    {
                        "name": "wasThrown",
                        "description": "True if the result was thrown during the evaluation.",
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "isOwn",
                        "description": "True if the property is owned for the object.",
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "symbol",
                        "description": "Property symbol object, if the property is of the `symbol` type.",
                        "optional": true,
                        "$ref": "RemoteObject"
                    }
                ]
            },
            {
                "id": "InternalPropertyDescriptor",
                "description": "Object internal property descriptor. This property isn't normally visible in JavaScript code.",
                "type": "object",
                "properties": [
                    {
                        "name": "name",
                        "description": "Conventional property name.",
                        "type": "string"
                    },
                    {
                        "name": "value",
                        "description": "The value associated with the property.",
                        "optional": true,
                        "$ref": "RemoteObject"
                    }
                ]
            },
            {
                "id": "PrivatePropertyDescriptor",
                "description": "Object private field descriptor.",
                "experimental": true,
                "type": "object",
                "properties": [
                    {
                        "name": "name",
                        "description": "Private property name.",
                        "type": "string"
                    },
                    {
                        "name": "value",
                        "description": "The value associated with the private property.",
                        "optional": true,
                        "$ref": "RemoteObject"
                    },
                    {
                        "name": "get",
                        "description": "A function which serves as a getter for the private property,\nor `undefined` if there is no getter (accessor descriptors only).",
                        "optional": true,
                        "$ref": "RemoteObject"
                    },
                    {
                        "name": "set",
                        "description": "A function which serves as a setter for the private property,\nor `undefined` if there is no setter (accessor descriptors only).",
                        "optional": true,
                        "$ref": "RemoteObject"
                    }
                ]
            },
            {
                "id": "CallArgument",
                "description": "Represents function call argument. Either remote object id `objectId`, primitive `value`,\nunserializable primitive value or neither of (for undefined) them should be specified.",
                "type": "object",
                "properties": [
                    {
                        "name": "value",
                        "description": "Primitive value or serializable javascript object.",
                        "optional": true,
                        "type": "any"
                    },
                    {
                        "name": "unserializableValue",
                        "description": "Primitive value which can not be JSON-stringified.",
                        "optional": true,
                        "$ref": "UnserializableValue"
                    },
                    {
                        "name": "objectId",
                        "description": "Remote object handle.",
                        "optional": true,
                        "$ref": "RemoteObjectId"
                    }
                ]
            },
            {
                "id": "ExecutionContextId",
                "description": "Id of an execution context.",
                "type": "integer"
            },
            {
                "id": "ExecutionContextDescription",
                "description": "Description of an isolated world.",
                "type": "object",
                "properties": [
                    {
                        "name": "id",
                        "description": "Unique id of the execution context. It can be used to specify in which execution context\nscript evaluation should be performed.",
                        "$ref": "ExecutionContextId"
                    },
                    {
                        "name": "origin",
                        "description": "Execution context origin.",
                        "type": "string"
                    },
                    {
                        "name": "name",
                        "description": "Human readable name describing given context.",
                        "type": "string"
                    },
                    {
                        "name": "uniqueId",
                        "description": "A system-unique execution context identifier. Unlike the id, this is unique across\nmultiple processes, so can be reliably used to identify specific context while backend\nperforms a cross-process navigation.",
                        "experimental": true,
                        "type": "string"
                    },
                    {
                        "name": "auxData",
                        "description": "Embedder-specific auxiliary data.",
                        "optional": true,
                        "type": "object"
                    }
                ]
            },
            {
                "id": "ExceptionDetails",
                "description": "Detailed information about exception (or error) that was thrown during script compilation or\nexecution.",
                "type": "object",
                "properties": [
                    {
                        "name": "exceptionId",
                        "description": "Exception id.",
                        "type": "integer"
                    },
                    {
                        "name": "text",
                        "description": "Exception text, which should be used together with exception object when available.",
                        "type": "string"
                    },
                    {
                        "name": "lineNumber",
                        "description": "Line number of the exception location (0-based).",
                        "type": "integer"
                    },
                    {
                        "name": "columnNumber",
                        "description": "Column number of the exception location (0-based).",
                        "type": "integer"
                    },
                    {
                        "name": "scriptId",
                        "description": "Script ID of the exception location.",
                        "optional": true,
                        "$ref": "ScriptId"
                    },
                    {
                        "name": "url",
                        "description": "URL of the exception location, to be used when the script was not reported.",
                        "optional": true,
                        "type": "string"
                    },
                    {
                        "name": "stackTrace",
                        "description": "JavaScript stack trace if available.",
                        "optional": true,
                        "$ref": "StackTrace"
                    },
                    {
                        "name": "exception",
                        "description": "Exception object if available.",
                        "optional": true,
                        "$ref": "RemoteObject"
                    },
                    {
                        "name": "executionContextId",
                        "description": "Identifier of the context where exception happened.",
                        "optional": true,
                        "$ref": "ExecutionContextId"
                    },
                    {
                        "name": "exceptionMetaData",
                        "description": "Dictionary with entries of meta data that the client associated\nwith this exception, such as information about associated network\nrequests, etc.",
                        "experimental": true,
                        "optional": true,
                        "type": "object"
                    }
                ]
            },
            {
                "id": "Timestamp",
                "description": "Number of milliseconds since epoch.",
                "type": "number"
            },
            {
                "id": "TimeDelta",
                "description": "Number of milliseconds.",
                "type": "number"
            },
            {
                "id": "CallFrame",
                "description": "Stack entry for runtime errors and assertions.",
                "type": "object",
                "properties": [
                    {
                        "name": "functionName",
                        "description": "JavaScript function name.",
                        "type": "string"
                    },
                    {
                        "name": "scriptId",
                        "description": "JavaScript script id.",
                        "$ref": "ScriptId"
                    },
                    {
                        "name": "url",
                        "description": "JavaScript script name or url.",
                        "type": "string"
                    },
                    {
                        "name": "lineNumber",
                        "description": "JavaScript script line number (0-based).",
                        "type": "integer"
                    },
                    {
                        "name": "columnNumber",
                        "description": "JavaScript script column number (0-based).",
                        "type": "integer"
                    }
                ]
            },
            {
                "id": "StackTrace",
                "description": "Call frames for assertions or error messages.",
                "type": "object",
                "properties": [
                    {
                        "name": "description",
                        "description": "String label of this stack trace. For async traces this may be a name of the function that\ninitiated the async call.",
                        "optional": true,
                        "type": "string"
                    },
                    {
                        "name": "callFrames",
                        "description": "JavaScript function name.",
                        "type": "array",
                        "items": {
                            "$ref": "CallFrame"
                        }
                    },
                    {
                        "name": "parent",
                        "description": "Asynchronous JavaScript stack trace that preceded this stack, if available.",
                        "optional": true,
                        "$ref": "StackTrace"
                    },
                    {
                        "name": "parentId",
                        "description": "Asynchronous JavaScript stack trace that preceded this stack, if available.",
                        "experimental": true,
                        "optional": true,
                        "$ref": "StackTraceId"
                    }
                ]
            },
            {
                "id": "UniqueDebuggerId",
                "description": "Unique identifier of current debugger.",
                "experimental": true,
                "type": "string"
            },
            {
                "id": "StackTraceId",
                "description": "If `debuggerId` is set stack trace comes from another debugger and can be resolved there. This\nallows to track cross-debugger calls. See `Runtime.StackTrace` and `Debugger.paused` for usages.",
                "experimental": true,
                "type": "object",
                "properties": [
                    {
                        "name": "id",
                        "type": "string"
                    },
                    {
                        "name": "debuggerId",
                        "optional": true,
                        "$ref": "UniqueDebuggerId"
                    }
                ]
            }
        ],
        "commands": [
            {
                "name": "enable",
                "description": "Enables reporting of execution contexts creation by means of `executionContextCreated` event.\nWhen the reporting gets enabled the event will be sent immediately for each existing execution\ncontext."
            },
            {
                "name": "evaluate",
                "description": "Evaluates expression on global object.",
                "parameters": [
                    {
                        "name": "expression",
                        "description": "Expression to evaluate.",
                        "type": "string"
                    },
                    {
                        "name": "objectGroup",
                        "description": "Symbolic group name that can be used to release multiple objects.",
                        "optional": true,
                        "type": "string"
                    },
                    {
                        "name": "includeCommandLineAPI",
                        "description": "Determines whether Command Line API should be available during the evaluation.",
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "silent",
                        "description": "In silent mode exceptions thrown during evaluation are not reported and do not pause\nexecution. Overrides `setPauseOnException` state.",
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "contextId",
                        "description": "Specifies in which execution context to perform evaluation. If the parameter is omitted the\nevaluation will be performed in the context of the inspected page.\nThis is mutually exclusive with `uniqueContextId`, which offers an\nalternative way to identify the execution context that is more reliable\nin a multi-process environment.",
                        "optional": true,
                        "$ref": "ExecutionContextId"
                    },
                    {
                        "name": "returnByValue",
                        "description": "Whether the result is expected to be a JSON object that should be sent by value.",
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "generatePreview",
                        "description": "Whether preview should be generated for the result.",
                        "experimental": true,
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "userGesture",
                        "description": "Whether execution should be treated as initiated by user in the UI.",
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "awaitPromise",
                        "description": "Whether execution should `await` for resulting value and return once awaited promise is\nresolved.",
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "throwOnSideEffect",
                        "description": "Whether to throw an exception if side effect cannot be ruled out during evaluation.\nThis implies `disableBreaks` below.",
                        "experimental": true,
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "timeout",
                        "description": "Terminate execution after timing out (number of milliseconds).",
                        "experimental": true,
                        "optional": true,
                        "$ref": "TimeDelta"
                    },
                    {
                        "name": "disableBreaks",
                        "description": "Disable breakpoints during execution.",
                        "experimental": true,
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "replMode",
                        "description": "Setting this flag to true enables `let` re-declaration and top-level `await`.\nNote that `let` variables can only be re-declared if they originate from\n`replMode` themselves.",
                        "experimental": true,
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "allowUnsafeEvalBlockedByCSP",
                        "description": "The Content Security Policy (CSP) for the target might block 'unsafe-eval'\nwhich includes eval(), Function(), setTimeout() and setInterval()\nwhen called with non-callable arguments. This flag bypasses CSP for this\nevaluation and allows unsafe-eval. Defaults to true.",
                        "experimental": true,
                        "optional": true,
                        "type": "boolean"
                    },
                    {
                        "name": "uniqueContextId",
                        "description": "An alternative way to specify the execution context to evaluate in.\nCompared to contextId that may be reused across processes, this is guaranteed to be\nsystem-unique, so it can be used to prevent accidental evaluation of the expression\nin context different than intended (e.g. as a result of navigation across process\nboundaries).\nThis is mutually exclusive with `contextId`.",
                        "experimental": true,
                        "optional": true,
                        "type": "string"
                    }
                ],
                "returns": [
                    {
                        "name": "result",
                        "description": "Evaluation result.",
                        "$ref": "RemoteObject"
                    },
                    {
                        "name": "exceptionDetails",
                        "description": "Exception details.",
                        "optional": true,
                        "$ref": "ExceptionDetails"
                    }
                ]
            }
        ],
        "events": [
            {
                "name": "consoleAPICalled",
                "description": "Issued when console API was called.",
                "parameters": [
                    {
                        "name": "type",
                        "description": "Type of the call.",
                        "type": "string",
                        "enum": [
                            "log",
                            "debug",
                            "info",
                            "error",
                            "warning",
                            "dir",
                            "dirxml",
                            "table",
                            "trace",
                            "clear",
                            "startGroup",
                            "startGroupCollapsed",
                            "endGroup",
                            "assert",
                            "profile",
                            "profileEnd",
                            "count",
                            "timeEnd"
                        ]
                    },
                    {
                        "name": "args",
                        "description": "Call arguments.",
                        "type": "array",
                        "items": {
                            "$ref": "RemoteObject"
                        }
                    },
                    {
                        "name": "executionContextId",
                        "description": "Identifier of the context where the call was made.",
                        "$ref": "ExecutionContextId"
                    },
                    {
                        "name": "timestamp",
                        "description": "Call timestamp.",
                        "$ref": "Timestamp"
                    },
                    {
                        "name": "stackTrace",
                        "description": "Stack trace captured when the call was made. The async stack chain is automatically reported for\nthe following call types: `assert`, `error`, `trace`, `warning`. For other types the async call\nchain can be retrieved using `Debugger.getStackTrace` and `stackTrace.parentId` field.",
                        "optional": true,
                        "$ref": "StackTrace"
                    },
                    {
                        "name": "context",
                        "description": "Console context descriptor for calls on non-default console context (not console.*):\n'anonymous#unique-logger-id' for call on unnamed context, 'name#unique-logger-id' for call\non named context.",
                        "experimental": true,
                        "optional": true,
                        "type": "string"
                    }
                ]
            }, {
                "name": "executionContextCreated",
                "description": "Issued when new execution context is created.",
                "parameters": [
                    {
                        "name": "context",
                        "description": "A newly created execution context.",
                        "$ref": "ExecutionContextDescription"
                    }
                ]
            }]
    }]
}

// you can add your custom debug command like this.
protocol.domains.push({
    "domain": "PuerhDebug",

    // something from devtool to jsengine
    "commands": [
        {
            "name": "enthroneAsHost",
            "parameters": [
                {
                    "name": "callID",
                    "type": "number"
                }
            ]
        },
        {
            "name": "returnHostCall",
            "parameters": [
                {
                    "name": "callID",
                    "type": "number"
                },
                {
                    "name": "error",
                    "type": "string"
                },
                {
                    "name": "result",
                    "type": "string"
                }
            ]
        }
    ],

    // something from jsengine to devtool
    "events": [{
        "name": "hostCalled",
        "parameters": [
            {
                "name": "action",
                "type": "string"
            },
            {
                "name": "callID",
                "type": "number"
            },
            {
                "name": "params",
                "type": "string"
            },
        ]
    }]
})

module.exports = protocol