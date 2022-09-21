try {
    winccoanodejs = require('./build/Release/winccoanodejs.node');
} catch (e) {
    var winccoanodejs = require('./build/Debug/winccoanodejs.node');
}

function dpConfig(dp) {
    const parts = dp.split(':');
    // if no ':' separators then there is no config
    if (parts.length === '1')
        return null;
    const config = parts.slice(-1)[0];
    if (config.slice(0, 1) !== '_')
        return null;
    return config;
}

function dpGet(dp, cb) {
    let promise;
    if (typeof cb !== 'function') {
        promise = new Promise((resolve, reject) => {
            cb = function(err, rs) {
                if (err)
                    return reject(new Error(err));
                return resolve(rs);
            };
        });
    }
    try {
        if (!dp) throw new Error('dpGet: dp provided is invalid');
        const config = dpConfig(dp);
        if (!config)
            dp = dp + ':_online.._value';
        winccoanodejs.dpGet(dp, cb);
    } catch (err) {
        cb(err);
    }

    return promise;
}

function dpSet(dp, value, cb) {
    let promise;
    if (typeof cb !== 'function') {
        promise = new Promise((resolve, reject) => {
            cb = function(err, rs) {
                if (err)
                    return reject(new Error(err));
                return resolve(rs);
            };
        });
    }

    try {
        if (!dp) throw new Error('dpSet: dp provided is invalid');
        const config = dpConfig(dp);
        if (!config)
            dp = dp + ':_original.._value';
        winccoanodejs.dpSet(dp, value, cb);
    } catch (err) {
        cb(err);
    }

    return promise;
}

function dpConnect(dp, cb, maxCallCount = 0) {
    let promise;
    if (typeof cb !== 'function') {
        promise = new Promise((resolve, reject) => {
            cb = function(err, rs) {
                if (err)
                    reject(new Error(err));
                else 
                    resolve(rs);
                dpDisconnect(dp, cb);
            };
        });
    }

    try {
        if (!dp)
            throw new Error('dpConnect: dp provided is invalid');
        const config = dpConfig(dp);
        if (!config)
            dp = dp + ':_online.._value';
        winccoanodejs.dpConnect(dp, cb, maxCallCount);
    } catch (err) {
        cb(err);
    }

    return promise;
}

function dpDisconnect(dp, connectcb, cb) {
    let promise;
    if (typeof cb !== 'function') {
        promise = new Promise((resolve, reject) => {
            cb = function(err, rs) {
                if (err)
                    return reject(new Error(err));
                return resolve(rs);
            };
        });
    }

    try {
        if (!dp)
            throw new Error('dpDisconnect: dp provided is invalid');
        const config = dpConfig(dp);
        if (!config)
            dp = dp + ':_online.._value';
        winccoanodejs.dpDisconnect(dp, connectcb);
        cb();
    } catch (err) {
        cb(err);
    }

    return promise;
}

function dpQuery(query, cb) {
    let promise;
    if (typeof cb !== 'function') {
        promise = new Promise((resolve, reject) => {
            cb = function(err, rs) {
                if (err)
                    return reject(new Error(err));
                return resolve(rs);
            };
        });
    }

    try {
        if (!query) throw new Error('dpQuery: query provided is invalid');
        winccoanodejs.dpQuery(query, (err, rs) => {
            if (err)
                return cb(err);
            if (rs)
                return cb(null, rs[1] || []);
            return cb();
        });
    } catch (err) {
        cb(err);
    }

    return promise;
}

function dpElementType(element, cb) {
    let promise;
    if (typeof cb !== 'function') {
        promise = new Promise((resolve, reject) => {
            cb = function(err, rs) {
                if (err)
                    return reject(new Error(err));
                return resolve(rs);
            };
        });
    }

    try {
        if (!element)
            throw new Error('dpElementType: element provided is invalid');
        winccoanodejs.dpElementType(element, cb);
    } catch (err) {
        cb(err);
    }

    return promise;
}

function getTypeName(dp, sys) {
    if (!dp) throw new Error('getTypeName: dp provided is invalid');
    if (!sys) {
        const parts = dp.split(':');
        if (parts.length === 2) {
            dp = parts[1];
            sys = parts[0];
        } else
            throw new TypeError('sys not set');
    }

    return winccoanodejs.getTypeName(dp, sys);
}

function getDpSys(dp, systems) {
    if (!dp)
        throw new Error('getDpSys: asset provided is invalid');
    if (!Array.isArray(systems))
        systems = [systems];
    for (let i in systems) {
        let sys = systems[i];
        if (winccoanodejs.dpExists(sys + dp))
            return sys;
    }
    throw new Error(`Could not get system for DP ${dp}`);
}

module.exports = {
    ...winccoanodejs,
    dpGet,
    dpSet,
    dpConnect,
    dpDisconnect,
    dpQuery,
    dpElementType,
    getTypeName,
    getDpSys
};
