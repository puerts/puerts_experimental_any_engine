export default function renderDeclaration(usageCollector) {
    let ret = 'declare namespace SC {\n\n';
    const allClsDict = usageCollector.getAllUsedCls();
    for (const cls of allClsDict) {
        ret += renderClass(cls, usageCollector) + '\n\n';
    }
    const allEnumDict = usageCollector.getAllUsedEnum();
    for (const enumm of allEnumDict) {
        ret += renderEnum(enumm, usageCollector) + '\n\n';
    }
    return ret + "\n\n}";
}
function addParamPrefix(type) {
    if (type.name == 'va_list')
        return '...$';
    return '$'; // prevent name to be JS keyword
}
function renderTypeName(type) {
    const name = type.name;
    if (name == 'va_list')
        return 'any[]';
    return cppFullNameToTsFullName(name);
}
function renderEnum(enumm, usageCollector) {
    let ret = ``;
    const nsAndClsName = cppFullNameToTsNSAndClsName(enumm.cppFullName);
    if (nsAndClsName.namespaces.length != 0) {
        ret += nsAndClsName.namespaces.map((ns) => {
            return `namespace ${ns} {\n`;
        }).join('\n');
    }
    ret += `const enum ${enumm.name} {\n`;
    Object.keys(enumm.enums).forEach(enumName => {
        ret += `    ${enumName} = ${enumm.enums[enumName]},\n`;
    });
    ret += '}\n';
    if (nsAndClsName.namespaces.length != 0) {
        ret += nsAndClsName.namespaces.map((ns) => {
            return `\n}`;
        }).join('\n');
    }
    return ret;
}
function renderClass(cls, usageCollector) {
    let ret = '';
    let baseClsStr = '';
    if (cls.baseTypeCppFullName) {
        const baseAstCls = usageCollector.findAstClass(cls.baseTypeCppFullName);
        if (baseAstCls) {
            if (baseAstCls.IsAbstract && baseAstCls.Destructors.Count == 0)
                baseClsStr = ' implements ' + cppFullNameToTsFullName(cls.baseTypeCppFullName);
            else if (baseAstCls.TemplateKind == CS.CppAst.CppTemplateKind.NormalClass)
                baseClsStr = ' extends ' + cppFullNameToTsFullName(cls.baseTypeCppFullName);
            // todo support non normal class
        }
    }
    const nsAndClsName = cppFullNameToTsNSAndClsName(cls.cppFullName);
    if (nsAndClsName.namespaces.length != 0) {
        ret += nsAndClsName.namespaces.map((ns) => {
            return `namespace ${ns} {\n`;
        }).join('\n');
    }
    ret += `class ${nsAndClsName.className}${baseClsStr} { \n`;
    if (cls.ctor.overloads.length != 0) {
        ret += cls.ctor.overloads.map((overload) => {
            ret = `    constructor(${overload.params.map((param) => {
                return `${addParamPrefix(param.type)}${param.name}: ${renderTypeName(param.type)}${param.defaultExpressionTS ? ' = ' + param.defaultExpressionTS : ''}`;
            }).join(', ')});\n`;
            ret += `    static [Symbol.alloc](${overload.params.map((param) => {
                return `${addParamPrefix(param.type)}${param.name}: ${renderTypeName(param.type)}${param.defaultExpressionTS ? ' = ' + param.defaultExpressionTS : ''}`;
            }).join(', ')}) : ${nsAndClsName.className};`;
            return ret;
        }).join('\n');
        ret += '\n\n';
    }
    if (cls.functions.length != 0) {
        ret += cls.functions.map((func) => {
            // debug && console.log('func:', func.name)
            return func.overloads.map((overload) => {
                return `    public ${overload.isStatic ? 'static ' : ''}${fixInvalidMemberName(overload.name)}(${overload.params.map((param) => {
                    // debug && console.log(param.name, 'fn default param:', param.defaultExpressionTS)
                    let name = param.name;
                    if (!name || (name.length == 1 && name.charCodeAt(0) == 1))
                        name = "_";
                    return `${addParamPrefix(param.type)}${name}: ${renderTypeName(param.type)}${param.defaultExpressionTS ? ' = ' + param.defaultExpressionTS : ''}`;
                }).join(', ')}) : ${renderTypeName(overload.returnType)};`;
            }).join('\n');
        }).join('\n\n');
        ret += '\n\n';
    }
    if (cls.fields.length != 0) {
        ret += cls.fields.map((field) => {
            // debug && console.log('field:', field.name)
            return `    public ${field.isStatic ? 'static ' : ''}${fixInvalidMemberName(field.name)}: ${renderTypeName(field.type)};`;
        }).join('\n\n');
        ret += '\n\n';
    }
    ret += '    [Symbol.dispose](): any; \n';
    ret += '\n}';
    if (nsAndClsName.namespaces.length != 0) {
        ret += nsAndClsName.namespaces.map((ns) => {
            return `\n}`;
        }).join('\n');
    }
    return ret;
}
function fixInvalidMemberName(name) {
    if (name.match(/[\=\*\+\-\/\<\>\s\[\]]/g))
        return `['${name}']`;
    return name;
}
function cppFullNameToTsFullName(name) {
    return name.replace(/::/g, '.');
}
function cppFullNameToTsNSAndClsName(name) {
    if (name.indexOf('::') == -1) {
        return { namespaces: [], className: name };
    }
    else {
        const parts = name.split('::');
        return { namespaces: parts.slice(0, parts.length - 1), className: parts[parts.length - 1] };
    }
}
