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
