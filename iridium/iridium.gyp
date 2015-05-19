{
	'targets': [
		{
			'target_name': 'trknotify',
			'type': 'static_library',
			'sources': [
				'trknotify.cpp',
				'trknotify.h',
				'trkbar.cpp',
				'trkbar.h',
			],
			'include_dirs': [
				'..',
				'../third_party/skia/include/config',
				'../third_party/skia/include/core',
			],
		},
	],
}
