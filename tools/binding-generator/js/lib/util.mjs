function csForEach(csList, callback) {
    const count = csList.Count;
    for (let i = 0; i < count; i++) {
        const item = csList.get_Item(i);
        callback(item, i);
    }
}
function csMap(csList, callback) {
    const count = csList.Count;
    const res = [];
    for (let i = 0; i < count; i++) {
        const item = csList.get_Item(i);
        res.push(callback(item, i));
    }
    return res;
}
function csWhere(csList, callback) {
    const count = csList.Count;
    for (let i = 0; i < count; i++) {
        const item = csList.get_Item(i);
        if (callback(item, i))
            return true;
    }
    return false;
}
export { csForEach, csMap, csWhere };
export function checkExclude(def, exclude) {
    if (def.constructor == CS.CppAst.CppField || def.constructor == CS.CppAst.CppFunction) {
        return !!exclude.members.filter((fullName) => {
            const tempSplit = fullName.split('::');
            const ns = tempSplit.slice(0, -1).join('::');
            const name = tempSplit[tempSplit.length - 1];
            return def.Name == name && def.FullParentName == ns;
        }).length;
    }
    else if (def.constructor == CS.CppAst.CppClass) {
        if (exclude.types.indexOf(def.FullName) != -1)
            return true;
        if (exclude.namespaces.filter((ns) => {
            if (def.FullName.indexOf(ns) != -1)
                return true;
        }).length)
            return true;
    }
}
