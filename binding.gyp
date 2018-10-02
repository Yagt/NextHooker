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
		'<(module_root_dir)/Builds/x86-Release/Build/vnrhook'
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")"
      ],
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