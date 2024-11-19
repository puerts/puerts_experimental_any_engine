(function () {
    const g = typeof globalThis == 'undefined' ? global : globalThis;
    const puer = g.puer || (g.puer = {});
    puer.v8module = puer.v8module || {};

    const V8ModuleWrap = loadCppType("PuerhTypescript::V8ModuleWrap");
    const moduleCacheBySpecifier = {}
    const moduleCacheByHash = {}
    const moduleResolvePromise = {}

    async function loadModule(resolvedModuleName) {
        let loadedContent = await new Promise((resolve, reject) => {
            __puerExternalHost.LoadFromJS(resolvedModuleName, (content) => {
                content === null || content === undefined ? reject(new Error(`module ${resolvedModuleName} not found`)) : resolve(content)
            })
        })

        const mod = new V8ModuleWrap()
        __puerInitModuleWrap(mod, resolvedModuleName, loadedContent);

        const ref = mod.getModuleName();
        const length = mod.getModuleRequestsLength();
        const proms = [];
        for (let i = 0; i < length; i++) {
            const request = mod.getModuleRequest(i);
            const resolvedModuleName = __puerExternalHost.Resolve(request, ref);
            proms.push(loadModuleFromCacheOrLoader(resolvedModuleName))
        }
        await Promise.all(proms);

        return mod;
    }
    async function loadModuleFromCacheOrLoader(resolvedModuleName) {
        if (moduleCacheBySpecifier[resolvedModuleName]) {
            return moduleCacheBySpecifier[resolvedModuleName]

        } else if (!moduleResolvePromise[resolvedModuleName]) {
            moduleResolvePromise[resolvedModuleName] = loadModule(resolvedModuleName);
            const mod = await moduleResolvePromise[resolvedModuleName]
            delete moduleResolvePromise[resolvedModuleName];
            moduleCacheBySpecifier[resolvedModuleName] = mod;
            moduleCacheByHash[mod.getModuleHash()] = mod;
            return mod;

        } else {
            return moduleResolvePromise[resolvedModuleName]
        }
    }

    return {
        v8ImportModule: async function (specifier, referrer) {
            const resolvedModuleName = __puerExternalHost.Resolve(specifier, referrer || "");
            const module = await loadModuleFromCacheOrLoader(resolvedModuleName)

            return await __puerEvaluateModuleByModuleWrap(module);
        },
        v8GetModuleByNameAndRef: function (specifier, referrer) {
            const resolvedSpecifier = __puerExternalHost.Resolve(specifier, referrer || "");
            return moduleCacheBySpecifier[resolvedSpecifier];
        },
        v8GetModuleByIdentityHash: function (hash) {
            return moduleCacheByHash[hash];
        }
    }
})();
