{
  'targets': [
    {
      'target_name': 'nexthooker',
      'sources': [
        'binding.cc',
        'nodewrapper/wrapper_api.cc',
		'nodewrapper/misc.cc'
      ],
      'libraries': [
		'<(module_root_dir)/Builds/Debug/x86/vnrhook'
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
		"<(module_root_dir)/include/"
      ],
	  "defines": [ "_UNICODE", "UNICODE" ],
      'configurations': {
        'Debug': {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'RuntimeLibrary': '3',
			  'AdditionalOptions': [
                '/std:c++17'
              ]
            }
          }
        },
        'Release': {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'RuntimeLibrary': '2',
			  'AdditionalOptions': [
                '/std:c++17'
              ]
            }
          }
        }
      }
    }
  ]
}