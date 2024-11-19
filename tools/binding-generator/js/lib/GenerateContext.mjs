import { csForEach } from "./util.mjs";
import { TSClass, TSEnum } from "./definitions.mjs";
export default class GenerateContext {
    constructor(compilation, refExclude, genExclude, specialTSName) {
        this.compilation = compilation;
        this.usedCls = new Map();
        this.usedEnum = new Map();
        this.specialTSNames = {};
        this.refExclude = refExclude || { types: [], members: [], namespaces: [] };
        this.refExclude.types = this.refExclude.types || [];
        this.refExclude.members = this.refExclude.members || [];
        this.refExclude.namespaces = this.refExclude.namespaces || [];
        this.genExclude = genExclude || { types: [], members: [], namespaces: [] };
        this.genExclude.types = this.genExclude.types || [];
        this.genExclude.members = this.genExclude.members || [];
        this.genExclude.namespaces = this.genExclude.namespaces || [];
        this.specialTSNames = specialTSName || {};
    }
    findAstNamespace(namespaces) {
        let findingName = namespaces.shift();
        let currentNamespace = this.compilation;
        while (namespaces.length != 0) {
            let matchedNamespace = null;
            if (currentNamespace.Namespaces)
                csForEach(currentNamespace.Namespaces, (item) => {
                    if (item.Name == findingName)
                        matchedNamespace = item;
                });
            if (!matchedNamespace && currentNamespace.Classes)
                csForEach(currentNamespace.Classes, (cls) => {
                    if (cls.Name == findingName)
                        matchedNamespace = cls;
                });
            if (!matchedNamespace) {
                console.warn(`find ${findingName} failed`);
                return null;
            }
            currentNamespace = matchedNamespace;
            findingName = namespaces.shift();
        }
        return {
            ns: currentNamespace,
            lastItemName: findingName
        };
    }
    findAstEnum(enumFullName) {
        let result = null;
        const nsResult = this.findAstNamespace(enumFullName.split("::"));
        if (!nsResult)
            return null;
        const { ns, lastItemName } = nsResult;
        csForEach(ns.Enums, (cls) => {
            if (cls.Name == lastItemName)
                result = cls;
        });
        return result;
    }
    findAstClass(clsFullName) {
        let result = null;
        const nsResult = this.findAstNamespace(clsFullName.split("::"));
        if (!nsResult)
            return null;
        const { ns, lastItemName } = nsResult;
        csForEach(ns.Classes, (cls) => {
            if (cls.Name == lastItemName)
                result = cls;
        });
        return result;
    }
    getAllUsedCls() {
        return this.usedCls.values();
    }
    getAllUsedEnum() {
        return this.usedEnum.values();
    }
    getAllHeaders() {
        const headers = [];
        for (const astCls of this.usedCls.keys()) {
            let header = astCls.SourceFile;
            // sometime the class will define as incomplete type. so we need to find the first member to get the real header path
            if (astCls.Fields.Count) {
                header = astCls.Fields.get_Item(0).SourceFile;
            }
            else if (astCls.Functions.Count) {
                header = astCls.Functions.get_Item(0).SourceFile;
            }
            else if (astCls.Constructors.Count) {
                header = astCls.Constructors.get_Item(0).SourceFile;
            }
            if (header)
                headers.push(header);
        }
        return headers;
    }
    addRefUsage(name) {
        const astClass = this.findAstClass(name);
        if (astClass) {
            if (!this.usedCls.has(astClass)) {
                if (TSClass.isNotSupportedClass(this, astClass))
                    return;
                let tsCls = new TSClass(this, astClass);
                this.usedCls.set(astClass, tsCls);
                return tsCls;
                // template not supported yet
                // if (astClass.TemplateKind == CS.CppAst.CppTemplateKind.TemplateSpecializedClass) {
                //     csForEach(astClass.TemplateSpecializedArguments, item => {
                //         if (item.TypeKind == CS.CppAst.CppTypeKind.Function) return;
                //         this.addRefUsage(item.FullName);
                //     })
                // }
            }
            return;
        }
        const astEnum = this.findAstEnum(name);
        if (astEnum) {
            if (!this.usedEnum.has(astEnum)) {
                let tsEnum = new TSEnum(this, astEnum);
                this.usedEnum.set(astEnum, tsEnum);
                // return tsEnum
            }
            return;
        }
        console.warn(`can't find class ${name} in compilation`);
    }
    expandCurrentUsage() {
        for (const cls of this.usedCls.values()) {
            cls.baseTypeCppFullName && this.addRefUsage(cls.baseTypeCppFullName);
            cls.ctor.overloads.forEach((func) => {
                const type = func.returnType.rawType;
                if (!type.isPrimitive)
                    this.addRefUsage(type.cppName);
                func.params.forEach((param) => {
                    const type = param.type.rawType;
                    if (!type.isPrimitive)
                        this.addRefUsage(type.cppName);
                    if (param.astField.InitExpression && param.astField.InitExpression.Kind == CS.CppAst.CppExpressionKind.DeclRef) {
                        const dvCls = this.addRefUsage(param.astField.InitExpression.toString().split('::').slice(0, -1).join('::'));
                        if (dvCls)
                            dvCls.addMember(param.astField.InitExpression.toString().split('::').slice(-1)[0]);
                    }
                });
            });
            cls.fields.forEach((field) => {
                const type = field.type.rawType;
                if (!type.isPrimitive)
                    this.addRefUsage(type.cppName);
            });
            cls.functions.forEach((overload) => {
                overload.overloads.forEach((func) => {
                    const type = func.returnType.rawType;
                    if (!type.isPrimitive)
                        this.addRefUsage(type.cppName);
                    func.params.forEach((param) => {
                        const type = param.type.rawType;
                        if (!type.isPrimitive)
                            this.addRefUsage(type.cppName);
                        if (param.astField.InitExpression && param.astField.InitExpression.Kind == CS.CppAst.CppExpressionKind.DeclRef) {
                            const dvCls = this.addRefUsage(param.astField.InitExpression.toString().split('::').slice(0, -1).join('::'));
                            if (dvCls)
                                dvCls.addMember(param.astField.InitExpression.toString().split('::').slice(-1)[0]);
                        }
                    });
                });
            });
        }
    }
    addBaseUsage(signature) {
        const name = signature.split('::').slice(0, -1).join('::');
        const astClass = this.findAstClass(name);
        if (astClass) {
            if (!this.usedCls.has(astClass)) {
                if (TSClass.isNotSupportedClass(this, astClass))
                    return;
                this.usedCls.set(astClass, new TSClass(this, astClass));
            }
            const cls = this.usedCls.get(astClass);
            if (signature.endsWith("::*")) {
                cls.addAllMember();
            }
            else {
                cls.addMember(signature);
            }
            return;
        }
        const astEnum = this.findAstEnum(name);
        if (astEnum) {
            if (!this.usedEnum.has(astEnum)) {
                this.usedEnum.set(astEnum, new TSEnum(this, astEnum));
            }
            return;
        }
        console.warn(`can't find class ${name} in compilation`);
    }
    findAllClass() {
        this.iterateNamespace(this.compilation);
    }
    iterateNamespace(namespace) {
        if (namespace.Classes)
            csForEach(namespace.Classes, (astClass) => {
                if (!this.usedCls.has(astClass)) {
                    if (TSClass.isNotSupportedClass(this, astClass))
                        return;
                    const tsCls = new TSClass(this, astClass);
                    this.usedCls.set(astClass, tsCls);
                    tsCls.addAllMember();
                }
                this.iterateNamespace(astClass);
            });
        if (!(namespace instanceof CS.CppAst.CppClass) && namespace.Namespaces)
            csForEach(namespace.Namespaces, (item) => {
                this.iterateNamespace(item);
            });
    }
}
