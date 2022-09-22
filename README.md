# WinCC OA Node.JS Connector
## Description
Use this connector to write Node.JS code for calls into WinCC OA. 
You could use this for a web application that uses WinCC OA functions
or any other Node.JS application. It achieves this by exposing the C++
API manager functions through JavaScript code.

This has been tested and functional with V3.15, V3.16 and V3.17 of WinCC OA only. V3.18 may work but use at your own risk.

The code connects as an API manager. You will need API manager licences for production use.

## Installation
Requirements:

WinCC OA V3.15-17 installed prior<br>
npm install winccoanodejs

## Usage
Require the package in your script, then you must connect to the WinCC OA server
before making and calls to WinCC OA.

    const winccoanodejs = require('winccoanodejs');
    if(winccoanodejs.connect('-currproj -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');
    const val = await winccoanodejs.dpGet('System1:ExampleDP.');
    console.log(val);

## API Reference
### Connection Functions
* [connect](#connect)
* [disconnect](#disconnect)
### Data Point Functions
* [dpGet](#dpGet)
* [dpSet](#dpSet)
* [dpConnect](#dpConnect)
* [dpDisconnect](#dpDisconnect)
* [dpQuery](#dpQuery)
* [dpElementType](#dpElementType)
* [dpExists](#dpExists)
* [getTypeName](#getTypeName)
* [dpGetDescription](#dpGetDescription)
* [dpGetUnit](#dpGetUnit)
* [dpGetFormat](#dpGetFormat)
* [dpNames](#dpNames)
### User Functions
* [getUserId](#getUserId)
* [checkPassword](#checkPassword)
### Alert Functions
* [alertConnect](#alertConnect)
* [alertSet](#alertSet)

<a id="connect"></a>
#### connect 
Connects your Node.JS application to the WinCC OA Server

*NB: This function is blocking*

##### Synopsis
    bool connect(string params, [string appName])

##### Parameter

| Parameter | Meaning |
------------|----------
| params    | The command line params you would send to a manager. <br>E.G. -proj Test -num 2 -event 192.168.1.1 -data 192.168.1.1 |
| appName   | *optional* - the application name. This will appear in the log viewer |

##### Return Value
The function returns true if the connection was successful

##### Example
    const winccoanodejs = require('winccoanodejs');
    if(winccoanodejs.connect('-currproj -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

<a id="disconnect"></a>
#### disconnect 
Disconnects your application from the WinCC OA Server.

*NB: this is the same as disconnecting a manager and the API will also quit your application*

##### Synopsis
    void disconnect()

##### Example
    const winccoanodejs = require('winccoanodejs');
    if(winccoanodejs.connect('-currproj -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    winccoanodejs.disconnect();

<a id="dpGet"></a>
#### dpGet 
Gets the value of a data point. This implementation can only read one data point at a time.

##### Synopsis
    Promise<any> dpGet(string dpe, [function callback])

##### Parameter

| Parameter | Meaning |
|---|---|
| dpe | Data point attributes to be read. If no config is specified for the data point the the default of _online.._value is used|
| callback | *optional* - a callback with the signature function(err, value). <br> If omitted then a Promise is returned.|

##### Return value
If successful and the `callback` variable is omitted then a `Promise` is returned that can be used with async calls.

##### Example
    const {connect, dpGet} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    const dp = 'System1.ExampleDP_Arg1.';

    async function main() {
        try {
            var value = await dpGet(dp);
            console.log(`${dp} value ${value}`);
        } catch(err) { console.error(err); }
    }

    main();

<a id="dpSet"></a>
#### dpSet 
Sets that value of a data point. This implementation can only write one data point at a time.

##### Synopsis
    Promise<void> dpSet(string dpe, anytype value, [function callback])

##### Parameter

| Parameter | Meaning |
|---|---|
| dpe | Data point attributes to be written. If no config is specified for the data point the the default of _original.._value is used|
| value | The value to set the data point to. If the data point type does not match the value type a cast will be attempted.|
| callback | *optional* - a callback with the signature function(err). <br> If omitted then a Promise is returned.|

##### Return value
If successful and the `callback` variable is omitted then a `Promise` is returned that can be used with async calls.

##### Example
    const {connect, dpSet} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    const dp = 'System1.ExampleDP_Arg1.';
    const value = 25.2;

    async function main() {
        try {
            await dpSet(dp, value);
            console.log(`${dp} value set to ${value}`);
        } catch(err) { console.error(err); }
    }

    main();

<a id="dpConnect"></a>
#### dpConnect 
Connects to a datapoint and calls a function whenever the data point value changes

##### Synopsis
    Promise<any> dpConnect(string dpe, [function callback], [int maxCallCount = 0])

##### Parameter

| Parameter | Meaning |
|---|---|
| dpe | Data point attributes to connect to. If no config is specified for the data point the the default of _online.._value is used|
| callback | *optional* - a callback with the signature function(err, value). This could be called multiple times until [dpDisconnect](#dpDisconnect) is called|
| maxCallCount | *optional* - default = 0. If set to non zero then the `callback` will only be called the number of times specified|

##### Return value
If the `callback` variable is omitted then a `Promise` is returned otherwise no value is returned

##### Details
If the `callback` variable is omitted then dpConnect will call dpDisconnect directly after the first change because the Promise cannot be
called multiple times.

When a callback is supplied the connection will persist and the function will continue to receive updates
until [dpDisconnect](#dpDisconnect) is called.

##### Example
    const {connect, dpConnect} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    const dp = 'System1:ExampleDP_Arg1.';

    function connectCallback(err, value) {
        if(err)
            return console.error(err);
        console.log(`${new Date().toLocaleString()} - ${dp} value ${value}`);
    }

    dpConnect(dp, connectCallback);

<a id="dpDisconnect"></a>
#### dpDisconnect 
Disconnects a data point from a previous [dpConnect](#dpConnect) call

##### Synopsis
    Promise<void> dpDisconnect(string dpe, function connectCallback, [function callback])

##### Parameter

| Parameter | Meaning |
|---|---|
| dpe | Data point attributes to disconnect from. If no config is specified for the data point the the default of _online.._value is used |
| connectCallback | The callback function used in the [dpConnect](#dpConnect) call |
| callback | *optional* - a callback with the signature function(err). Will be called when dpDisconnect is complete. |

##### Return value
If the `callback` variable is omitted then a `Promise` is returned otherwise no value is returned

##### Example
    const {connect, dpConnect, dpDisconnect} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    const dp = 'System1:ExampleDP_Arg1.';

    function connectCallback(err, value) {
        if(err)
            return console.error(err);
        console.log(`${new Date().toLocaleString()} - ${dp} value ${value}`);
    }

    dpConnect(dp, connectCallback);

    setTimeout(async () => {
        try {
            await dpDisconnect(dp, connectCallback);
            console.log(`Disconnected from ${dp}`);
        } catch (err) {
            console.error(err);
        }
    }, 10000);

<a id="dpQuery"></a>
#### dpQuery 
Retrieves attribute values with the help of SQL-like statements. See qthelp://wincc_oa/doc/SQL/Query-05.htm
in the WinCC OA Documentation for query structure.

##### Synopsis
    Promise<Array> dpQuery(string query, [function callback])

##### Parameter

| Parameter | Meaning |
|---|---|
| query | The SQL-like query for WinCC OA |
| callback | *optional* - a callback with the signature function(err, table). `table` has the results from the query in a tabular array. First row is the column headers. |

##### Return value
If the `callback` variable is omitted then a `Promise` is returned otherwise no value is returned

##### Example
    const {connect, dpQuery} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    /*
    Example output
    [
        [ '', ':_online.._value', ':_online.._stime' ],
        [ 'System1:ExampleDP_Arg1.', 4, 2022-09-12T05:20:26.473Z ],
        [ 'System1:ExampleDP_Rpt1.', 0, 1970-01-01T00:00:00.000Z ],
        [ 'System1:ExampleDP_Rpt2.', 0, 1970-01-01T00:00:00.000Z ],
        [ 'System1:ExampleDP_Arg2.', 50, 2022-09-12T04:14:21.062Z ],
        [ 'System1:ExampleDP_Rpt3.', 0, 1970-01-01T00:00:00.000Z ],
        [ 'System1:ExampleDP_Rpt4.', 0, 1970-01-01T00:00:00.000Z ],
        [ 'System1:ExampleDP_Result.', 54, 2022-09-12T05:20:26.473Z ],
        [ 'System1:ExampleDP_Trend1.', 0.947601318359375, 2022-09-12T04:14:21.062Z]
    ]
    */

    async function main() {
        try {
            const table = await dpQuery(`SELECT '_online.._value', '_online.._stime' FROM '*ExampleDP*' WHERE _DPT="ExampleDP_Float"`);
            console.log(table);
        } catch(err) {
            console.error(err);
        }
    }

    main();

<a id="dpElementType"></a>
#### dpElementType 
Gets the integer value for a dp elements type. Type numbers can be found in the API header file `DpElementType.hxx`

##### Synopsis
    Promise<number> dpElementType(string element, [function callback])

##### Parameter

| Parameter | Meaning |
|---|---|
| element | The the datapoint element to get the type for |
| callback | *optional* - a callback with the signature function(err, type). |

##### Return Value
If the `callback` variable is omitted then a `Promise` is returned otherwise no value is returned

##### Example
    const {connect, dpElementType} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    const type = await dpElementType('System1:ExampleDP_Arg1.');
    console.log(type); //Output: 22

<a id="getTypeName"></a>
#### getTypeName 
Return the type name of a data point or data point element as a string. This function is blocking.

##### Synopsis
    string getTypeName(string dp, [string sys])

##### Parameter

| Parameter | Meaning |
|---|---|
| dp | The data point or data point element to get the type name for. |
| sys | *optional* The name of the system to where the data point exists. This parameter is optional if you include the system in the `dp` parameter. E.G. `System1:ExampleDP_Arg1.` |

##### Return Value
Returns the strring name of the data point. E.G. float, ExampleDP_Float

##### Example
    const {connect, getTypeName} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    const typeName = getTypeName('ExampleDP_Arg1.','System1');
    console.log(typeName); //OUTPUT: ExampleDP_Float

<a id="dpExists"></a>
#### dpExists 
Checks if a datapoint or element exists

##### Synopsis
    bool dpExists(string dp)

##### Parameter

| Parameter | Meaning |
|---|---|
| dp | The data point or data point element to check. If the system is ommitted from the dp then the local system is assumed. |

##### Return Value
Returns true if the datapoint or element exists. Otherwise false.

##### Example
    const {connect, dpExists} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    //OUTPUT: true false
    console.log(dpExists('ExampleDP_Arg1.'), dpExists('ExampleDP_Arg0.'));

<a id="getDpSys"></a>
#### getDpSys 
Checks a list of systems for a given datapoint. Will return the first system that it finds the data point on.

##### Synopsis
    string getDpSys(string dp, Array<string> systems)

##### Parameter

| Parameter | Meaning |
|---|---|
| dp | The data point or data point element to check. Do not include a system prefix. |
| systems | Array of systems to check in for the data point. |

##### Return Value
Returns the system that the data point exists in. If the data point is not found in any system then an error is thrown.

##### Example
    const {connect, getDpSys} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    //OUTPUT: 'System1:'
    console.log(getDpSys('ExampleDP_Arg1.', ['System2:', 'System1:']));

    //OUTPUT: Uncaught Error: Could not get system for DP ExampleDP_Arg0.
    getDpSys('ExampleDP_Arg0.', ['System2:','System1:']);

<a id="getUserId"></a>
#### getUserId 
Gets the integer user id value for a given username

##### Synopsis
    int getUserId(string username)

##### Parameter

| Parameter | Meaning |
|---|---|
| username | The username of the user that you need to know the UserID for. |

##### Return Value
Returns the integer value for the UserID. If the user does not exist then it returns 65535

##### Example
    const {connect, getUserId} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    //OUTPUT: 0 65535
    console.log(getUserId('root'), getUserId('test'));

<a id="checkPassword"></a>
#### checkPassword 
Checks the password for a given UserID. Use [getUserId](#getUserId) if you have the Username instead of the UserID

##### Synopsis
    bool checkPassword(int userId, string password)

##### Parameter

| Parameter | Meaning |
|---|---|
| userId | The UserID of the password to check. |
| password | The password in plain text. *Care when using this that the password is not transmitted over unencrypted channels* |

##### Return Value
Returns true is the password matches. False otherwise.

##### Example
    const {connect, checkPassword} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');
    
    //OUTPUT: true false - only if default password for root used in example project.
    //user 0 = root
    console.log(checkPassword(0, ''), checkPassword(0, '1234'));

<a id="alertConnect"></a>
#### alertConnect 
Connect to an alert value and execute a callback when the alert changes.

##### Synopsis
    bool alertConnect(string alertAttr1, [string alertAttr2, ...], function callback)

##### Parameter

| Parameter | Meaning |
|---|---|
| alertAttr1, [alertAttr2, ...] | _alert_hdl config attributes to connect to. See _alert_hdl in documentation for valid attributes. |
| callback | The callback to execute on alert changes with signature `function(err, Array<any>)` |

##### Example
    const {connect, alertConnect} = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    function cb(err, values) {
        if(err) {
            console.error(err);
        } else {
            console.log(values);
        }
    }

    //OUTPUT: true
    console.log(alertConnect(':_alert_hdl.._text', ':_alert_hdl.._act_state', cb);
    //OUTPUT: when System1:ExampleDP_AlertHdl1. is set to TRUE
    /*
    [
        'System1:ExampleDP_AlertHdl1.:_alert_hdl.._text',
        'Value to 1',
        'System1:ExampleDP_AlertHdl1.:_alert_hdl.._ack_type',
        3
    ]
    */

<a id="alertSet"></a>
#### alertSet 
Set attributes of alarms in a similar way to [dpSet](#dpSet).

##### Synopsis
    bool alertSet(string dpe, date alertTime, any value)

##### Parameter

| Parameter | Meaning |
|---|---|
| dpe | The data point element with an alert that you want to set values of. |
| alertTime | The time of the alert that you want to set the value for.|
| value | The value to set on the attribute |

##### Return Value
Return true if the set was successful. False otherwise.

##### Example
    //This example requires ExampleDP_AlertHdl1. to be set to TRUE and unacknowledged
    const {connect, alertSet, dpQuery } = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    async function main() {
        const tab = await dpQuery(`SELECT ALERT '_alert_hdl.._came_time' FROM 'ExampleDP_AlertHdl1.'`);
        const time = tab[1][1];
        //_ack_state constants
        //DPATTR_ACKTYPE_NOT = 0
        //DPATTR_ACKTYPE_MULTIPLE = 1
        //DPATTR_ACKTYPE_SINGLE = 2
        alertSet('System1:ExampleDP_AlertHdl1.:_alert_hdl.._ack_state', time, 2);
    }

    main();

<a id="dpGetDescription"></a>
#### dpGetDescription 
Get the data point description from the _common config. This function is blocking.

##### Synopsis
    string dpGetDescription(string dpe)

##### Parameter

| Parameter | Meaning |
|---|---|
| dpe | The data point element that you want to get the description for. |

##### Return Value
A string with the description

##### Example
    //This example requires you to set a description in the _common config for ExampleDP_Arg1.
    const {connect, dpGetDescription } = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    //OUTPUT: Description for ExampleDP_Arg1
    console.log(dpGetDescription('ExampleDP_Arg1.'));

<a id="dpGetUnit"></a>
#### dpGetUnit 
Get the data point unit from the _common config. This function is blocking.

##### Synopsis
    string dpGetUnit(string dpe)

##### Parameter

| Parameter | Meaning |
|---|---|
| dpe | The data point element that you want to get the units for. |

##### Return Value
A string with the units

##### Example
    //This example requires you to set a unit in the _common config for ExampleDP_Arg1.
    const {connect, dpGetUnit } = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    //OUTPUT: °C
    console.log(dpGetUnit('ExampleDP_Arg1.'));

<a id="dpGetFormat"></a>
#### dpGetFormat 
Get the data point format from the _common config. This function is blocking.

##### Synopsis
    string dpGetFormat(string dpe)

##### Parameter

| Parameter | Meaning |
|---|---|
| dpe | The data point element that you want to get the format for. |

##### Return Value
A string with the format

##### Example
    //This example requires you to set a format in the _common config for ExampleDP_Arg1.
    const {connect, dpGetFormat } = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    //OUTPUT: %15.3f
    console.log(dpGetFormat('ExampleDP_Arg1.'));

<a id="dpNames"></a> 
#### dpNames
Get a list of data point names that match a pattern

##### Synopsis
    Array<string> dpNames(string pattern)

##### Parameter

| Parameter | Meaning |
|---|---|
| pattern | The data point pattern to search for. |

##### Return Value
Returns an array containing the data points that match the pattern

##### Example
    const {connect, dpNames } = require('winccoanodejs');
    if(connect('-proj Example -num 2', 'my-nodejs-app'))
        console.log('WinCC OA Connected');

    const arr = dpNames('Example*');
    console.log(arr);
    /* OUTPUT
    [
        'System1:ExampleDP_AlertHdl1',
        'System1:ExampleDP_AlertHdl2',
        'System1:ExampleDP_Arg1',
        'System1:ExampleDP_Arg2',
        'System1:ExampleDP_BarTrend',
        'System1:ExampleDP_DDE',
        'System1:ExampleDP_Result',
        'System1:ExampleDP_Rpt1',
        'System1:ExampleDP_Rpt2',
        'System1:ExampleDP_Rpt3',
        'System1:ExampleDP_Rpt4',
        'System1:ExampleDP_SumAlert',
        'System1:ExampleDP_Trend1',
        'System1:ExampleDP_bt'
    ]*/