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
        '<(module_root_dir)/Builds/Debug/Debug/vnrhost'
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")"
      ],
      'configurations': {
        'Debug': {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'RuntimeLibrary': '3'
            }
          }
        },
        'Release': {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'RuntimeLibrary': '2'
            }
          }
        }
      }
    }
  ]
}