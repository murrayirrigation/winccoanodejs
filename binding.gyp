{
    "targets": [{
        "target_name": "winccoanodejs",
        "sources": [
            "cppsrc/main.cpp",
			"cppsrc/manager/ManagerManager.cxx",
			"cppsrc/manager/ManagerResources.cxx",
			"cppsrc/manager/AnswerValue.cpp",
            "cppsrc/CallbackProcessor.cpp",
			"cppsrc/AnswerCallback.cpp",
			"cppsrc/ConnectAnswerCallback.cpp",
            "cppsrc/AlertConnectCallback.cpp"
        ],
        "include_dirs": [
            "<!(node -e \"require('nan')\")",
            "<!(node -e \"require('./winccpath')\")/include/Basics/Utilities",
            "<!(node -e \"require('./winccpath')\")/include/Messages",
            "<!(node -e \"require('./winccpath')\")/include/Manager",
            "<!(node -e \"require('./winccpath')\")/include/Basics/DpBasics",
            "<!(node -e \"require('./winccpath')\")/include/Basics/Utilities",
            "<!(node -e \"require('./winccpath')\")/include/Basics/Variables",
            "<!(node -e \"require('./winccpath')\")/include/Basics/NoPosix",
            "<!(node -e \"require('./winccpath')\")/include/OaBasics/Core",
            "<!(node -e \"require('./winccpath')\")/include/OaBasics/Utilities",
            "<!(node -e \"require('./winccpath')\")/include/BCMNew",
            "<!(node -e \"require('./winccpath')\")/include/Datapoint",
            "<!(node -e \"require('./winccpath')\")/include/Configs",
            "<!(node -e \"require('./winccpath')\")/include/winnt"
        ],
        "libraries": [
            "<!(node -e \"require('./winccpath')\")/lib.winnt/bcm.lib",
            "<!(node -e \"require('./winccpath')\")/lib.winnt/ewo.lib",
            "<!(node -e \"require('./winccpath')\")/lib.winnt/libBasics.lib",
            "<!(node -e \"require('./winccpath')\")/lib.winnt/libComDrv.lib",
            "<!(node -e \"require('./winccpath')\")/lib.winnt/libConfigs.lib",
            "<!(node -e \"require('./winccpath')\")/lib.winnt/libCtrl.lib",
            "<!(node -e \"require('./winccpath')\")/lib.winnt/libDatapoint.lib",
            "<!(node -e \"require('./winccpath')\")/lib.winnt/libManager.lib",
            "<!(node -e \"require('./winccpath')\")/lib.winnt/libMessages.lib",
            "<!(node -e \"require('./winccpath')\")/lib.winnt/libV24.lib"
        ],
        "conditions": [
            [
                "OS=='win'",
                {
                    "configurations": {
                        "Debug": {
                            "msvs_settings": {
                                "VCCLCompilerTool": {
                                    "ForcedIncludeFiles": "<!(node -e \"console.log(process.cwd())\")/dllspec.h",
                                    "ExceptionHandling": "2"
                                }
                            }
                        },
						"Release": {
                            "msvs_settings": {
                                "VCCLCompilerTool": {
                                    "ForcedIncludeFiles": "<!(node -e \"console.log(process.cwd())\")/dllspec.h",
                                    "ExceptionHandling": "2"
                                }
                            }
                        }
                    }
                }
            ]
        ]
    }]
}