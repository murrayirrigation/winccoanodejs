const path = require('path');

var dir = process.env.PATH.split(';').filter(t => t.indexOf('WinCC_OA') !== -1);

if (dir.length === 0) {
    console.log('');
} else {
    var parts = dir[0].split('\\').slice(0, -2);
    parts.push('api');
    console.log(path.join.apply(null, parts));
}