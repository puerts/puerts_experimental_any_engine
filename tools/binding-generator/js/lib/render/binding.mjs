import { checkExclude } from "../util.mjs";
function renderBinding(generateContext, bindingName, BindingConfig) {
    const allClsDict = generateContext.getAllUsedCls();
    const allCls = [];
    for (const cls of allClsDict) {
        allCls.push(cls);
    }
    const allHeaders = generateContext.getAllHeaders();
    return {
        header: `
#pragma once
#ifndef PUER_BINDING_GENERATING
${(function () {
            const includePaths = BindingConfig.includePaths;
            return allHeaders
                .map((header) => {
                let converted = false;
                header = header.replace(/\\/g, '/');
                includePaths.forEach((includePath) => {
                    if (header.startsWith(includePath) && !converted) {
                        header = header.replace(includePath, '');
                        if (header.startsWith('/'))
                            header = header.slice(1);
                        converted = true;
                    }
                });
                return `#include "${header}"`;
            })
                .filter((val, index, arr) => {
                return arr.indexOf(val) == index;
            })
                .join('\n');
        })()}
${allCls.map((cls) => {
            return renderClassDeclare(cls, generateContext);
        }).join('')}
#define PUER_BINDING_GENERATED_${bindingName} 1
#endif
`,
        source: `
#include "${BindingConfig.output.puerHeaderName}"
#if !defined(PUER_BINDING_GENERATING) && defined(PUER_BINDING_GENERATED_${bindingName})
${allCls.map((cls) => {
            return renderClassOverloadWithDefaultValue(cls);
        }).join('')}
struct PuerAutoRegister_${bindingName}
{
    PuerAutoRegister_${bindingName}()
    {
        ${allCls.map((cls) => {
            return renderClassBinding(cls, generateContext);
        }).join('')}
    }
};
PuerAutoRegister_${bindingName} _PuerAutoRegister_${bindingName}_;
#endif
        `
    };
}
function renderClassDeclare(cls, generateContext) {
    if (checkExclude(cls.astClass, generateContext.genExclude))
        return '';
    return `\nUsingCppType(${cls.cppFullName})`;
}
function renderDefaultValues(params) {
    let stillHasDefault = true;
    return params
        .map((p) => p)
        .reverse()
        .map((param) => {
        if (stillHasDefault && param.defaultExpressionCpp) {
            return `, ${param.defaultExpressionCpp}`;
        }
        stillHasDefault = false;
        return '';
    })
        .reverse()
        .filter(str => str)
        .join('');
}
function renderClassOverloadWithDefaultValue(cls) {
    let ret = '';
    // ctor with default value is not supported by puerts
    // if (cls.ctor.overloads.length > 1) {
    //     cls.ctor.overloads.forEach((overload) => {
    //         const self = overload.isStatic ? "(*)" : `(${cls.fullname}::*)`
    //         const methodSign = `${overload.returnType.cppName}${self}(${overload.params.map(param => param.type.cppName).join(', ')})${overload.isConst ? ' const' : ''}`;
    //         ret += `DeclOverload(${cls.fullname}, ${methodSign}, &${cls.fullname}::${overload.name}${overload.params.map(param => {
    //             if (param.defaultValue) {
    //                 return `,  ${param.defaultValue}`
    //             }
    //             return '';
    //         }).filter(str => str).join('')})\n`
    //     })
    // }
    cls.functions.forEach((func) => {
        if (func.overloads.length > 1) {
            ret += `\nDeclOverloads(${cls.cppFullName.replace(/::/g, "_")}_${func.index})`;
            func.overloads.forEach((overload) => {
                const self = overload.isStatic ? "(*)" : `(${cls.cppFullName}::*)`;
                const methodSign = `${overload.returnType.cppName}${self}(${overload.params.map(param => param.type.cppName).join(', ')})${overload.isConst ? ' const' : ''}`;
                ret += `\nDeclOverload(${cls.cppFullName.replace(/::/g, "_")}_${func.index}, ${methodSign}, &${cls.cppFullName}::${overload.name}${renderDefaultValues(overload.params)})`;
            });
        }
    });
    return ret;
}
function renderClassBinding(cls, generateContext) {
    if (checkExclude(cls.astClass, generateContext.genExclude))
        return '';
    if (cls.astClass.IsAbstract && cls.astClass.Destructors.Count == 0)
        return '';
    let res = `
        PUERTS_NAMESPACE::DefineClass<${cls.cppFullName}>()\n`;
    if (cls.baseTypeCppFullName) {
        res += `            .Extends<${cls.baseTypeCppFullName}>()\n`;
    }
    if (cls.ctor.overloads.length == 0) {
    }
    else if (cls.ctor.overloads.length == 1) {
        const ctor = cls.ctor.overloads[0];
        // ignore params with default value
        if (ctor.params.filter(param => !param.defaultExpressionCpp).length == 0)
            res += `            .Constructor()\n`;
        else
            res += `            .Constructor<${ctor.params.map(param => param.type.cppName).join(', ')}>()\n`;
    }
    else {
        res += `            .Constructor(
                CombineConstructors(\n${cls.ctor.overloads.map((ctor) => {
            return `                    MakeConstructor(${cls.cppFullName}${ctor.params.map(param => ', ' + param.type.cppName).join('')})`;
        }).join(",\n")}
                )
            )\n`;
    }
    cls.functions.forEach((func) => {
        const self = func.isStatic ? "(*)" : `(${cls.cppFullName}::*)`;
        res += func.isStatic ? `            .Function("${func.name}", ` : `            .Method("${func.name}", `;
        if (func.overloads.length == 1) {
            const method = func.overloads[0];
            const methodSign = `${method.returnType.cppName}${self}(${method.params.map(param => param.type.cppName).join(', ')})${method.isConst ? ' const' : ''}`;
            res += `SelectFunction(${methodSign}, &${cls.cppFullName}::${method.name}${renderDefaultValues(method.params)}))\n`;
        }
        else {
            res += `CombineOverloads(${func.overloads.map((overload) => {
                const methodSign = `${overload.returnType.cppName}${self}(${overload.params.map(param => {
                    return param.type.cppName;
                }).join(', ')})${overload.isConst ? ' const' : ''}`;
                // return `\n        MakeOverload(${methodSign}, &${cls.fullname}::${method.name})`
                return `\n                SelectOverload(${cls.cppFullName.replace(/::/g, "_")}_${func.index}, ${methodSign})`;
            }).join(",")}\n            ))\n`;
        }
    });
    cls.fields.forEach((field) => {
        if (field.isStatic) {
            if (field.isReadOnly) { // static const
                if (field.astField.Type.TypeKind == CS.CppAst.CppTypeKind.Qualified &&
                    field.astField.Type.ElementType.TypeKind == CS.CppAst.CppTypeKind.Primitive)
                    res += `            .Variable("${field.name}", MakeConstVariable(${cls.cppFullName}::${field.name}))\n`;
                else
                    res += `            .Variable("${field.name}", MakeReadonlyVariable(&${cls.cppFullName}::${field.name}))\n`;
            }
            else {
                res += `            .Variable("${field.name}", MakeVariable(&${cls.cppFullName}::${field.name}))\n`;
            }
        }
        else {
            if (field.isReadOnly) {
                res += `            .Property("${field.name}", MakeReadonlyProperty(&${cls.cppFullName}::${field.name}))\n`;
            }
            else {
                res += `            .Property("${field.name}", MakeProperty(&${cls.cppFullName}::${field.name}))\n`;
            }
        }
    });
    res += `            .Register();`;
    return res;
}
export default renderBinding;
